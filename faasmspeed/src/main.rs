use anyhow::Result;
use argh::FromArgs;
use num_traits::Num;
use std::time::{Duration, Instant};
use tokio::io::{AsyncBufReadExt, AsyncWriteExt, BufReader};

#[derive(FromArgs, Debug, Clone)]
/// Latency testing for faasm function calls
struct Options {
    /// url of the faasm endpoint
    #[argh(option, short = 'h', default = "String::from(\"localhost:8080\")")]
    url: String,

    /// millisecond timeout for a single request
    #[argh(option, short = 'x', default = "10000")]
    timeout: u64,

    /// how many (on average) requests per second to submit
    #[argh(option, short = 'r', default = "10.0")]
    requests_per_second: f64,

    /// how many seconds to run the test for
    #[argh(option, short = 't', default = "10.0")]
    time: f64,

    /// number of threads to run
    #[argh(option, short = 'p', default = "2")]
    parallelism: usize,

    /// faasm function user
    #[argh(option, short = 'u', default = "String::from(\"demo\")")]
    user: String,

    /// faasm function name
    #[argh(option, short = 'f', default = "String::from(\"hello\")")]
    function: String,

    /// only do one function call
    #[argh(switch, short = 'o')]
    oneshot: bool,

    /// output in csv format
    #[argh(switch, short = 'c')]
    csv: bool,

    /// monitoring host[s] to connect to, separated by ;
    #[argh(option, short = 'm', default = "String::from(\"\")")]
    monitoring_hosts: String,

    /// faasm function input data pool (repeating, must be valid contents for a JSON string)
    #[argh(positional)]
    input_data: Vec<String>,
}

fn get_opts() -> Options {
    let mut opts: Options = argh::from_env();
    if opts.input_data.is_empty() {
        opts.input_data.push(String::new());
    }
    if !opts.url.starts_with("http://") {
        opts.url.insert_str(0, "http://");
    }
    opts
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord)]
struct RequestResult {
    us_request: u128,
    us_status: u128,
    num_bytes: usize,
    start_time: u128,
}

async fn async_main(opts: &'static Options) -> Result<()> {
    let client = make_client()?;
    eprintln!("[status] Created an HTTP client");
    let request_pool: Vec<_> = opts
        .input_data
        .iter()
        .map(|data| {
            make_request(
                &client,
                &opts.url,
                Box::leak(make_json_call(&opts.user, &opts.function, data).into_boxed_str()),
                opts.timeout,
            )
        })
        .collect::<Result<Vec<_>>>()?;
    eprintln!(
        "[status] Created a request pool of {} requests to {}.",
        request_pool.len(),
        opts.url
    );
    eprintln!("[status] Making a test request");
    {
        let test_request = request_pool[0].try_clone().unwrap();
        let pre_request = Instant::now();
        let response = client.execute(test_request).await?.error_for_status()?;
        let status = response.status();
        let bytes = response.bytes().await?;
        let duration = pre_request.elapsed();
        let bytecount = if opts.oneshot {
            bytes.len()
        } else {
            80.min(bytes.len())
        };
        let first_bytes = String::from_utf8_lossy(&bytes[0..bytecount]);
        eprintln!(
            "[status] Test request success status {} after {:.3} ms, first {} bytes of content: {}",
            status.as_str(),
            duration.as_micros() as f64 / 1000.0,
            first_bytes.len(),
            first_bytes,
        );
    }
    if opts.oneshot {
        return Ok(());
    }
    let mut mhosts = Vec::new();
    for host in opts
        .monitoring_hosts
        .split(';')
        .map(str::trim)
        .filter(|h| !h.is_empty())
    {
        eprintln!("[status] Connecting to monitoring host {}", host);
        let conn = tokio::net::TcpStream::connect(host).await?;
        conn.set_nodelay(true)?;
        mhosts.push(conn);
    }
    let total_requests = (opts.requests_per_second * opts.time).ceil() as i64;
    assert!(total_requests >= 0);
    eprintln!(
        "[status] Starting the request generator, will send {} requests in total",
        total_requests
    );
    for host in mhosts.iter_mut() {
        host.write_u8(b'[').await?;
    }
    let mut interval =
        tokio::time::interval(Duration::from_secs_f64(1.0 / opts.requests_per_second));
    let mut next_progress = Instant::now();
    let mut which_request = 0;
    let mut handles: Vec<tokio::task::JoinHandle<Result<RequestResult>>> =
        Vec::with_capacity(total_requests as usize);
    let reference_time = Instant::now();
    for request_id in 0..total_requests {
        interval.tick().await;
        let request = request_pool[which_request].try_clone().unwrap();
        let client_ref = client.clone();
        handles.push(tokio::spawn(async move {
            let start_time = reference_time.elapsed().as_micros();
            let pre_request = Instant::now();
            let response = client_ref.execute(request).await?.error_for_status()?;
            let t_status = pre_request.elapsed();
            let num_bytes = response.bytes().await?.len();
            let t_response = pre_request.elapsed();

            Ok(RequestResult {
                us_request: t_response.as_micros(),
                us_status: t_status.as_micros(),
                num_bytes,
                start_time,
            })
        }));
        tokio::task::yield_now().await;
        which_request += 1;
        if which_request == request_pool.len() {
            which_request = 0;
        }
        if Instant::now() >= next_progress {
            let pct = request_id * 100 / total_requests;
            eprint!(
                "\r[status] Progress: {:3}% {:8}/{:8}   ",
                pct, request_id, total_requests
            );
            next_progress = Instant::now() + Duration::from_millis(300);
        }
    }
    eprintln!(
        "\r[status] Progress: 100% {r:8}/{r:8}   ",
        r = total_requests
    );
    eprintln!("[status] Gathering results");
    let mut results = Vec::with_capacity(handles.len());
    let mut errors: Vec<anyhow::Error> = Vec::with_capacity(32);
    for handle in handles.into_iter() {
        match handle.await {
            Ok(Ok(r)) => results.push(r),
            Ok(Err(e)) => errors.push(e),
            Err(e) => errors.push(anyhow::Error::from(e).context("Join error")),
        }
    }
    let mut monitoring_results: Vec<String> = Vec::with_capacity(mhosts.len());
    let mut buf = String::with_capacity(8192);
    for host in mhosts.iter_mut() {
        host.write_u8(b']').await?;
        buf.clear();
        let (hread, mut hwrite) = host.split();
        BufReader::new(hread).read_line(&mut buf).await?;
        monitoring_results.push(buf.clone());
        hwrite.write_u8(b'q').await?;
        hwrite.shutdown().await?;
    }

    if opts.csv {
        println!(
            "opts,host,{},user,{},function,{},req-rps,{:.1},req-time,{:.1},input-data-0,{}",
            opts.url,
            opts.user,
            opts.function,
            opts.requests_per_second,
            opts.time,
            opts.input_data[0]
        );
        println!(
            "errors,count,{}\nresults,count,{}",
            errors.len(),
            results.len()
        );
        for mresult in monitoring_results {
            println!("monitor,{}", mresult.trim());
        }
    } else {
        println!(
            "{} errors encountered, statistics from {} results",
            errors.len(),
            results.len()
        );
        for mresult in monitoring_results {
            println!("Monitoring result: {}", mresult.trim());
        }
    }
    if !errors.is_empty() {
        for err in errors.iter().take(10) {
            eprintln!("{}", err);
        }
    }
    {
        results.sort_unstable_by_key(|r| r.start_time);
        for i in (1..results.len()).rev() {
            results[i].start_time -= results[i - 1].start_time;
        }
        if results.len() > 2 {
            results[0].start_time = results[1].start_time;
        }
        results.sort_unstable_by_key(|r| r.start_time);
        let avg = print_stats(
            opts.csv,
            results.iter().map(|r| r.start_time),
            "Interval between requests [µs]",
            "inter-request-us",
        );
        let rps = 1.0e6 / avg as f64;
        println!("avg-rps,avg-rps,{:.4}", rps);
    }

    results.sort_unstable_by_key(|r| r.us_request);
    print_stats(
        opts.csv,
        results.iter().map(|r| r.us_request),
        "Request latency [µs]",
        "latency-us",
    );
    results.sort_unstable_by_key(|r| r.us_status);
    print_stats(
        opts.csv,
        results.iter().map(|r| r.us_status),
        "Status latency [µs] (excluding body)",
        "status-us",
    );
    results.sort_unstable_by_key(|r| r.num_bytes);
    print_stats(
        opts.csv,
        results.iter().map(|r| r.num_bytes as u128),
        "Bytes sent [b]",
        "bytes-sent",
    );

    Ok(())
}

fn print_stats<I: ExactSizeIterator<Item = N> + Clone, N>(
    csv: bool,
    sorted_data: I,
    variable_name: &'static str,
    short_name: &'static str,
) -> N
where
    N: Copy
        + Num
        + std::fmt::Display
        + std::iter::Sum
        + std::convert::TryFrom<usize>
        + std::convert::TryInto<usize>,
{
    let zero = N::zero();
    let sum: N = sorted_data.clone().sum();
    let min: N = sorted_data.clone().next().unwrap_or(zero);
    let max: N = sorted_data.clone().last().unwrap_or(zero);
    let n = sorted_data.len();
    let q1: N = sorted_data.clone().nth(n / 4).unwrap_or(zero);
    let q2: N = sorted_data.clone().nth(n / 2).unwrap_or(zero);
    let q3: N = sorted_data.clone().nth(3 * n / 4).unwrap_or(zero);
    let q01: N = sorted_data.clone().nth(n / 100).unwrap_or(zero);
    let q99: N = sorted_data.clone().nth(99 * n / 100).unwrap_or(zero);
    let avg: N = sum / N::try_from(n.max(1)).unwrap_or_else(|_| N::one());
    let variance: N = sorted_data.map(|x| (x - avg) * (x - avg)).sum();
    let stddev = (variance.try_into().unwrap_or(0usize) as f64).sqrt();
    if csv {
        let fields: [(&'static str, &dyn std::fmt::Display); 9] = [
            ("min", &min),
            ("q1pct", &q01),
            ("q1", &q1),
            ("med", &q2),
            ("q3", &q3),
            ("q99pct", &q99),
            ("max", &max),
            ("avg", &avg),
            ("stddev", &stddev),
        ];
        for (field, value) in fields {
            println!("{},{},{:.1}", short_name, field, value);
        }
    } else {
        println!("{}", variable_name);
        println!(
            " min = {}  q1% = {}  q1 = {}  med = {}  q3 = {}  q99% = {}  max = {}",
            min, q01, q1, q2, q3, q99, max
        );
        println!(" avg = {}  stddev = {:.1}", avg, stddev);
    }
    avg
}

fn make_client() -> Result<reqwest::Client> {
    Ok(reqwest::ClientBuilder::new()
        .no_gzip()
        .no_brotli()
        .no_deflate()
        .referer(false)
        .no_proxy()
        .timeout(Duration::from_secs(2))
        .https_only(false)
        .build()?)
}

fn make_json_call(user: &str, function: &str, input_data: &str) -> String {
    format!(
        r#"{{"user":"{}","function":"{}","input_data":"{}"}}"#,
        user, function, input_data
    )
}

fn make_request(
    client: &reqwest::Client,
    url: &'static str,
    data: &'static str,
    timeout_ms: u64,
) -> Result<reqwest::Request> {
    Ok(client
        .request(reqwest::Method::POST, url)
        .header(
            reqwest::header::CONTENT_TYPE,
            reqwest::header::HeaderValue::from_static("application/json"),
        )
        .body(data)
        .timeout(Duration::from_millis(timeout_ms))
        .build()?)
}

fn main() -> Result<()> {
    let opts: &'static Options = Box::leak(Box::new(get_opts()));
    eprintln!("[status] Parsed options");
    let rt = if opts.parallelism <= 1 {
        tokio::runtime::Builder::new_current_thread()
    } else {
        tokio::runtime::Builder::new_multi_thread()
    }
    .thread_name("faasm-request-gen")
    .worker_threads(opts.parallelism)
    .max_blocking_threads(1)
    .enable_time()
    .enable_io()
    .build()?;
    rt.block_on(async_main(opts))?;
    rt.shutdown_timeout(Duration::from_secs(5));
    Ok(())
}
