use anyhow::Result;
use argh::FromArgs;
use itertools::Itertools;
use num_traits::Num;
use reqwest::Client;
use std::{
    sync::{
        atomic::{AtomicBool, AtomicI64, Ordering},
        Arc,
    },
    time::{Duration, Instant},
};
use tokio::{
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader},
    sync::Barrier,
    task::JoinHandle,
};

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
    #[argh(option, short = 'r')]
    requests_per_second: Vec<f64>,

    /// how many seconds to run the test for
    #[argh(option, short = 't', default = "10.0")]
    time: f64,

    /// number of threads to run
    #[argh(option, short = 'p', default = "4")]
    parallelism: usize,

    /// faasm function user
    #[argh(option, short = 'u', default = "String::from(\"demo\")")]
    user: String,

    /// faasm function name
    #[argh(option, short = 'f')]
    functions: Vec<String>,

    /// only do one function call
    #[argh(switch, short = 'o')]
    oneshot: bool,

    /// output in csv format
    #[argh(switch, short = 'c')]
    csv: bool,

    /// do not actually send HTTP requests
    #[argh(switch, short = 'd')]
    dry_run: bool,

    /// ndp offloading fraction numerator (X/...)
    #[argh(option, short = 'n')]
    offload_frac_num: Vec<i32>,

    /// ndp offloading fraction denominator (.../X)
    #[argh(option, short = 'N', default = "1")]
    offload_frac_den: i32,

    /// monitoring host[s] to connect to, separated by ;
    #[argh(option, short = 'm', default = "String::from(\"\")")]
    monitoring_hosts: String,

    /// faasm function input data pool (repeating, must be valid contents for a JSON string), for multiple functions use multiple @file.txt arguments
    #[argh(positional)]
    input_data: Vec<String>,
}

fn get_opts() -> Options {
    let mut opts: Options = argh::from_env();
    if opts.input_data.is_empty() {
        opts.input_data.push(String::new());
    }
    if opts.requests_per_second.is_empty() {
        opts.requests_per_second.push(10.0);
    }
    if opts.functions.is_empty() {
        opts.functions.push(String::from("hello"));
    }
    if opts.offload_frac_num.is_empty() {
        opts.offload_frac_num.push(1);
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

#[derive(Default)]
struct SharedState {
    total_requests: AtomicI64,
    sent_requests: AtomicI64,
    recv_requests: AtomicI64,
    ndp_requests: AtomicI64,
    done: AtomicBool,
}

async fn run_function(
    opts: &'static Options,
    idx: usize,
    client: Client,
    phase_barrier: Arc<Barrier>,
    shared_state: Arc<SharedState>,
) -> Vec<Result<RequestResult>> {
    let function: &str = opts
        .functions
        .get(idx)
        .expect("Function name not provided for one of the functions");
    let requests_per_second = opts
        .requests_per_second
        .get(idx)
        .expect("RPS not provided for one of the functions");
    let offload_frac_num = *opts
        .offload_frac_num
        .get(idx)
        .expect("Offload % not provided for one of the functions");
    let input_data: Vec<String> = if idx == 0 && !opts.input_data[0].starts_with('@') {
        opts.input_data.clone()
    } else {
        std::fs::read_to_string(
            opts.input_data
                .get(idx)
                .expect("Input not provided for one of the functions")
                .strip_prefix('@')
                .expect("Input dataset needs @")
                .trim(),
        )
        .expect("Couldn't read arguments file")
        .lines()
        .map(str::trim)
        .filter(|l| !l.is_empty())
        .map(String::from)
        .collect()
    };
    let force_ndp = offload_frac_num > opts.offload_frac_den;
    // (no ndp, ndp)
    let request_pool: Vec<_> = input_data
        .into_iter()
        .map(|data| {
            Ok((
                make_request(
                    &client,
                    &opts.url,
                    Box::leak(
                        make_json_call(&opts.user, function, &data, true, force_ndp)
                            .into_boxed_str(),
                    ),
                    opts.timeout,
                )?,
                make_request(
                    &client,
                    &opts.url,
                    Box::leak(
                        make_json_call(&opts.user, function, &data, false, force_ndp)
                            .into_boxed_str(),
                    ),
                    opts.timeout,
                )?,
            ))
        })
        .collect::<Result<Vec<_>>>()
        .expect("Couldn't create request list");
    eprintln!(
        "[status][{function}] Created a request pool of {} requests to {}.",
        request_pool.len(),
        opts.url
    );
    eprintln!("[status][{function}] Making a test request");
    if !opts.dry_run {
        let test_request = if offload_frac_num > 0 {
            &request_pool[0].1
        } else {
            &request_pool[0].0
        }
        .try_clone()
        .unwrap();
        let pre_request = Instant::now();
        let response = client
            .execute(test_request)
            .await
            .expect("Test request failure")
            .error_for_status()
            .expect("Test request status failure");
        let status = response.status();
        let bytes = response.bytes().await.expect("Test request bytes failure");
        let duration = pre_request.elapsed();
        let bytecount = if opts.oneshot {
            bytes.len()
        } else {
            60.min(bytes.len())
        };
        let first_bytes = String::from_utf8_lossy(&bytes[0..bytecount])
            .replace('\n', "\\\\")
            .replace(|c: char| !c.is_ascii_graphic(), "?");
        eprintln!(
            "[status][{function}] Test request success status {} after {:.3} ms, first {} bytes of content: {}",
            status.as_str(),
            duration.as_micros() as f64 / 1000.0,
            first_bytes.len(),
            first_bytes,
        );
    }
    // Sync: test requests
    phase_barrier.wait().await;
    if opts.oneshot {
        return vec![];
    }
    // Wait for monitoring to start
    phase_barrier.wait().await;

    let total_requests = (requests_per_second * opts.time).ceil() as i64;
    assert!(total_requests >= 0);
    eprintln!(
        "[status][{function}] Starting the request generator, will send {total_requests} requests in total",
    );
    shared_state
        .total_requests
        .fetch_add(total_requests, Ordering::AcqRel);
    // Sync: total request count
    phase_barrier.wait().await;

    let mut which_request = 0;
    let mut handles: Vec<tokio::task::JoinHandle<Result<RequestResult>>> =
        Vec::with_capacity(total_requests as usize);
    let reference_time = Instant::now();
    let mut ndp_var = opts.offload_frac_den - 1;
    let mut interval = tokio::time::interval(Duration::from_secs_f64(1.0 / requests_per_second));
    for _request_id in 0..total_requests {
        interval.tick().await;
        ndp_var += 1;
        while ndp_var > opts.offload_frac_den {
            ndp_var -= opts.offload_frac_den;
        }
        let allow_ndp = ndp_var <= offload_frac_num;
        let request = if allow_ndp || force_ndp {
            shared_state.ndp_requests.fetch_add(1, Ordering::AcqRel);
            &request_pool[which_request].1
        } else {
            &request_pool[which_request].0
        }
        .try_clone()
        .unwrap();
        let client_ref = client.clone();
        let state_ref = shared_state.clone();
        handles.push(tokio::spawn(async move {
            let start_time = reference_time.elapsed().as_micros();
            state_ref.sent_requests.fetch_add(1, Ordering::AcqRel);
            if !opts.dry_run {
                let pre_request = Instant::now();
                let response = client_ref.execute(request).await?.error_for_status()?;
                let t_status = pre_request.elapsed();
                let num_bytes = response.bytes().await?.len();
                let t_response = pre_request.elapsed();
                state_ref.recv_requests.fetch_add(1, Ordering::AcqRel);

                Ok(RequestResult {
                    us_request: t_response.as_micros(),
                    us_status: t_status.as_micros(),
                    num_bytes,
                    start_time,
                })
            } else {
                state_ref.recv_requests.fetch_add(1, Ordering::AcqRel);
                Ok(RequestResult {
                    us_request: 1000,
                    us_status: 1000,
                    num_bytes: 1024,
                    start_time,
                })
            }
        }));
        tokio::task::yield_now().await;
        which_request += 1;
        if which_request == request_pool.len() {
            which_request = 0;
        }
    }

    let mut results = Vec::with_capacity(handles.len());
    for jh in handles.into_iter() {
        results.push(jh.await.expect("Join error"));
    }

    phase_barrier.wait().await;
    results
}

async fn async_main(opts: &'static Options) -> Result<()> {
    let client = make_client()?;
    eprintln!("[status] Created an HTTP client");
    let phase_barrier = Arc::new(Barrier::new(opts.functions.len() + 1));
    let shared_state = Arc::new(SharedState::default());

    #[allow(clippy::needless_collect)]
    let func_tasks: Vec<JoinHandle<_>> = opts
        .functions
        .iter()
        .enumerate()
        .map(|(fidx, _fname)| {
            let phase_barrier = phase_barrier.clone();
            let client = client.clone();
            let shared_state = shared_state.clone();
            tokio::spawn(async move {
                run_function(opts, fidx, client, phase_barrier, shared_state).await
            })
        })
        .collect();

    let mut mhosts = Vec::with_capacity(opts.monitoring_hosts.len());
    if !opts.dry_run {
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
    }

    // Wait for test requests to complete
    phase_barrier.wait().await;
    if opts.oneshot {
        return Ok(());
    }

    for host in mhosts.iter_mut() {
        host.write_u8(b'[').await?;
    }

    // Sync: monitoring start
    phase_barrier.wait().await;

    // Sync: total request count
    phase_barrier.wait().await;

    let progress = tokio::spawn({
        let shared_state = shared_state.clone();
        async move {
            let mut progress_interval = tokio::time::interval(Duration::from_millis(200));
            while !shared_state.done.load(Ordering::Acquire) {
                progress_interval.tick().await;
                let total = shared_state.total_requests.load(Ordering::Acquire);
                let sent = shared_state.sent_requests.load(Ordering::Acquire);
                let recv = shared_state.recv_requests.load(Ordering::Acquire);
                eprint!(
                    "\r[status] Progress: {:3}% {:8}S / {:8}R / {:8}T   ",
                    recv * 100 / total,
                    sent,
                    recv,
                    total
                );
            }
            let total = shared_state.total_requests.load(Ordering::Acquire);
            eprintln!(
                "\r[status] Progress: 100% {r:8}S / {r:8}R / {r:8}T   ",
                r = total
            );
        }
    });

    // Sync: finish all requests
    phase_barrier.wait().await;
    shared_state.done.store(true, Ordering::Release);

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

    progress.await.unwrap();

    let total_requests = shared_state.total_requests.load(Ordering::Acquire) as usize;
    let mut results: Vec<Vec<_>> = std::iter::repeat_with(|| Vec::with_capacity(total_requests))
        .take(opts.functions.len())
        .collect();
    let mut errors: Vec<Vec<anyhow::Error>> = std::iter::repeat_with(|| Vec::with_capacity(32))
        .take(opts.functions.len())
        .collect();
    for (fidx, handle) in func_tasks.into_iter().enumerate() {
        match handle.await {
            Ok(r) => {
                for result in r {
                    match result {
                        Ok(rr) => {
                            results[fidx].push(rr);
                        }
                        Err(rr) => {
                            errors[fidx].push(rr);
                        }
                    }
                }
            }
            Err(e) => panic!("Join error: {}", e),
        }
    }

    if !errors.is_empty() {
        let mut err_strs: Vec<String> = errors
            .iter()
            .map(|es| es.iter())
            .flatten()
            .map(|e| format!("{e}"))
            .collect();
        err_strs.sort();
        for (err_str, errs) in &err_strs.iter().group_by(|a| *a) {
            let num = errs.count();
            eprintln!("{num}x {err_str}");
        }
    }
    for (fidx, (results, errors)) in results.iter_mut().zip(errors.iter_mut()).enumerate() {
        let function = &opts.functions[fidx];
        if opts.csv {
            println!(
                "opts,host,{},user,{},function,{},ndp,{:.2},first-ndp,{:.2},actual-ndp,{:.6},req-rps,{:.1},first-rps,{:.1},req-time,{:.1},input-data-0,{}",
                opts.url,
                opts.user,
                function,
                opts.offload_frac_num[fidx] as f64 / opts.offload_frac_den as f64,
                opts.offload_frac_num[0] as f64 / opts.offload_frac_den as f64,
                shared_state.ndp_requests.load(Ordering::Acquire) as f64 / total_requests as f64,
                opts.requests_per_second[fidx],
                opts.requests_per_second[0],
                opts.time,
                opts.input_data[0]
            );
            println!(
                "errors,count,{}\nresults,count,{}",
                errors.len(),
                results.len()
            );
            for mresult in monitoring_results.iter() {
                println!("monitor,{}", mresult.trim());
            }
        } else {
            println!("For function {function}:");
            println!(
                "{} errors encountered, statistics from {} results",
                errors.len(),
                results.len()
            );
            for mresult in monitoring_results.iter() {
                println!("Monitoring result: {}", mresult.trim());
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
                "Inter-request interval [µs]",
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
            "Status latency [µs]",
            "status-us",
        );
        results.sort_unstable_by_key(|r| r.num_bytes);
        print_stats(
            opts.csv,
            results.iter().map(|r| r.num_bytes as u128),
            "Bytes sent [b]",
            "bytes-sent",
        );
    }

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
    let cnt: N = N::try_from(n.max(1)).unwrap_or_else(|_| N::one());
    let avg: N = sum / cnt;
    let variance: N = sorted_data.map(|x| (x - avg) * (x - avg)).sum::<N>() / cnt;
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
        print!("{}", variable_name);
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

fn make_json_call(
    user: &str,
    function: &str,
    input_data: &str,
    forbid_ndp: bool,
    force_storage: bool,
) -> String {
    format!(
        r#"{{"user":"{}","function":"{}","input_data":"{}","forbid_ndp":{}{}}}"#,
        user,
        function,
        input_data,
        if forbid_ndp { "true" } else { "false" },
        if force_storage {
            ",\"is_storage\":true"
        } else {
            ""
        }
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
