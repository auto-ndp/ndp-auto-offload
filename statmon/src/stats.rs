use anyhow::{Context, Result};
use std::fmt::Write as _;
use std::fs::{File, OpenOptions};
use std::io::{prelude::*, SeekFrom};

use crate::cpustat::{CpuStat, CpuTotals};

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum StatValue {
    Float(f64),
    Int(i64),
}

impl StatValue {
    pub fn setf(&mut self, v: f64) {
        *self = Self::Float(v);
    }

    pub fn seti(&mut self, v: i64) {
        *self = Self::Int(v);
    }

    pub fn valf(&self) -> f64 {
        match self {
            Self::Float(v) => *v,
            Self::Int(v) => *v as f64,
        }
    }

    pub const fn minval(&self) -> Self {
        match self {
            Self::Float(_) => Self::Float(f64::MIN),
            Self::Int(_) => Self::Int(i64::MIN),
        }
    }

    pub const fn maxval(&self) -> Self {
        match self {
            Self::Float(_) => Self::Float(f64::MAX),
            Self::Int(_) => Self::Int(i64::MAX),
        }
    }

    pub const fn zeroval(&self) -> Self {
        match self {
            Self::Float(_) => Self::Float(0.0),
            Self::Int(_) => Self::Int(0),
        }
    }

    pub fn set_min(&mut self, other: Self) {
        match (*self, other) {
            (Self::Float(m), Self::Float(v)) => self.setf(m.min(v)),
            (Self::Int(m), Self::Int(v)) => self.seti(m.min(v)),
            _ => panic!("Mismatched statistic types"),
        }
    }

    pub fn set_max(&mut self, other: Self) {
        match (*self, other) {
            (Self::Float(m), Self::Float(v)) => self.setf(m.max(v)),
            (Self::Int(m), Self::Int(v)) => self.seti(m.max(v)),
            _ => panic!("Mismatched statistic types"),
        }
    }

    pub fn set_add(&mut self, other: Self) {
        match (*self, other) {
            (Self::Float(m), Self::Float(v)) => self.setf(m + v),
            (Self::Int(m), Self::Int(v)) => self.seti(m + v),
            _ => panic!("Mismatched statistic types"),
        }
    }
}

impl std::fmt::Display for StatValue {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Float(v) => v.fmt(f),
            Self::Int(v) => v.fmt(f),
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub enum WhichStat {
    LoadAvg1min = 0,
    MemTotal,
    MemAvailable,
    MemActive,
    MemInactive,
    MemAnonHugePages,
    CpumsUser,
    CpumsNice,
    CpumsSystem,
    CpumsIdle,
    CpumsIowait,
    CpumsIrq,
    CpumsSoftirq,
    ContextSwitches,
    ProcsRunning,
    ProcsBlocked,
    NetRxBytes,
    NetRxErrors,
    NetTxBytes,
    NetTxErrors,
    //
    TotalStatCount,
}

#[derive(Clone, Debug)]
pub struct Stat {
    pub name: &'static str,
    pub value_now: StatValue,
    pub value_min: StatValue,
    pub value_max: StatValue,
    pub value_sum: StatValue,
    pub value_cnt: i64,
}

impl Stat {
    const fn int(name: &'static str) -> Self {
        Self {
            name,
            value_now: StatValue::Int(0),
            value_min: StatValue::Int(0).maxval(),
            value_max: StatValue::Int(0).minval(),
            value_sum: StatValue::Int(0),
            value_cnt: 0,
        }
    }

    const fn float(name: &'static str) -> Self {
        Self {
            name,
            value_now: StatValue::Float(0.0),
            value_min: StatValue::Float(0.0).maxval(),
            value_max: StatValue::Float(0.0).minval(),
            value_sum: StatValue::Float(0.0),
            value_cnt: 0,
        }
    }

    pub fn reset(&mut self) {
        self.value_min = self.value_now.maxval();
        self.value_max = self.value_now.minval();
        self.value_sum = self.value_now.zeroval();
        self.value_cnt = 0;
    }

    pub fn update_aggregate(&mut self) {
        self.value_min.set_min(self.value_now);
        self.value_max.set_max(self.value_now);
        self.value_sum.set_add(self.value_now);
        self.value_cnt += 1;
    }
}

const STATS_INIT: [Stat; WhichStat::TotalStatCount as usize] = [
    Stat::float("load_avg1"),
    Stat::int("mem_total"),
    Stat::int("mem_available"),
    Stat::int("mem_active"),
    Stat::int("mem_inactive"),
    Stat::int("mem_anonhp"),
    Stat::int("cpums_user"),
    Stat::int("cpums_nice"),
    Stat::int("cpums_system"),
    Stat::int("cpums_idle"),
    Stat::int("cpums_iowait"),
    Stat::int("cpums_irq"),
    Stat::int("cpums_softirq"),
    Stat::int("cswitches"),
    Stat::int("procs_running"),
    Stat::int("procs_blocked"),
    Stat::int("net_rx_bytes"),
    Stat::int("net_rx_errors"),
    Stat::int("net_tx_bytes"),
    Stat::int("net_tx_errors"),
];

pub struct Stats {
    in_range: bool,
    pub host_prefix: String,
    pub net_iface: String,
    rdbuf: String,
    fp_loadavg: Option<File>,
    fp_meminfo: Option<File>,
    fp_net: Option<File>,
    cpu: CpuStat,
    stats: [Stat; WhichStat::TotalStatCount as usize],
    net_rx_ref: i64,
    net_rxe_ref: i64,
    net_tx_ref: i64,
    net_txe_ref: i64,
}

impl Stats {
    pub const fn new() -> Self {
        Self {
            in_range: false,
            host_prefix: String::new(),
            net_iface: String::new(),
            rdbuf: String::new(),
            fp_loadavg: None,
            fp_meminfo: None,
            fp_net: None,
            cpu: CpuStat::new(),
            stats: STATS_INIT,
            net_rx_ref: 0,
            net_rxe_ref: 0,
            net_tx_ref: 0,
            net_txe_ref: 0,
        }
    }

    pub fn begin_range(&mut self) {
        self.in_range = true;
        self.stats.iter_mut().for_each(Stat::reset);
    }

    pub fn end_range(&mut self) {
        self.in_range = false;
    }

    pub fn stat_packet(&self) -> String {
        let mut outbuf = String::with_capacity(8192);
        for stat in self.stats.iter() {
            let avg = stat.value_sum.valf() / stat.value_cnt.max(1) as f64;
            write!(
                &mut outbuf,
                "{hp}{n},{now},{hp}{n}_min,{min},{hp}{n}_max,{max},{hp}{n}_avg,{avg},{hp}{n}_sum,{sum},{hp}{n}_cnt,{cnt},",
                hp = self.host_prefix,
                n = stat.name,
                now = stat.value_now,
                min = stat.value_min,
                max = stat.value_max,
                sum = stat.value_sum,
                cnt = stat.value_cnt,
                avg = avg,
            )
            .unwrap();
        }
        outbuf.pop();
        outbuf.push('\n');
        outbuf
    }

    pub fn update(&mut self) -> Result<()> {
        if self.rdbuf.capacity() < 4096 {
            self.rdbuf.reserve(4096);
        }
        self.upd_loadavg()?;
        self.upd_meminfo()?;
        self.upd_net()?;
        self.cpu.update(&mut self.rdbuf)?;
        {
            let cpums = self.cpu.cpu_totals_ms();
            self.stats[WhichStat::CpumsUser as usize]
                .value_now
                .seti(cpums[CpuTotals::UserMode as usize] as i64);
            self.stats[WhichStat::CpumsNice as usize]
                .value_now
                .seti(cpums[CpuTotals::NiceUserMode as usize] as i64);
            self.stats[WhichStat::CpumsSystem as usize]
                .value_now
                .seti(cpums[CpuTotals::SystemMode as usize] as i64);
            self.stats[WhichStat::CpumsIdle as usize]
                .value_now
                .seti(cpums[CpuTotals::Idle as usize] as i64);
            self.stats[WhichStat::CpumsIowait as usize]
                .value_now
                .seti(cpums[CpuTotals::IoWait as usize] as i64);
            self.stats[WhichStat::CpumsIrq as usize]
                .value_now
                .seti(cpums[CpuTotals::Irq as usize] as i64);
            self.stats[WhichStat::CpumsSoftirq as usize]
                .value_now
                .seti(cpums[CpuTotals::SoftIrq as usize] as i64);
            self.stats[WhichStat::ContextSwitches as usize]
                .value_now
                .seti(self.cpu.context_switches() as i64);
            self.stats[WhichStat::ProcsRunning as usize]
                .value_now
                .seti(self.cpu.procs_running() as i64);
            self.stats[WhichStat::ProcsBlocked as usize]
                .value_now
                .seti(self.cpu.procs_io_blocked() as i64);
        }
        if self.in_range {
            self.stats.iter_mut().for_each(Stat::update_aggregate);
        }
        Ok(())
    }

    fn upd_loadavg(&mut self) -> Result<()> {
        if self.fp_loadavg.is_none() {
            self.fp_loadavg = Some(OpenOptions::new().read(true).open("/proc/loadavg")?);
        }
        let fp_loadavg = self.fp_loadavg.as_mut().unwrap();
        self.rdbuf.clear();
        fp_loadavg.read_to_string(&mut self.rdbuf)?;
        fp_loadavg.seek(SeekFrom::Start(0))?;
        let ldavg = self
            .rdbuf
            .split_once(' ')
            .context("Splitting 1min loadavg")?
            .0;
        self.stats[WhichStat::LoadAvg1min as usize]
            .value_now
            .setf(ldavg.parse().context("Parsing 1min loadavg")?);
        Ok(())
    }

    // line -> (var, bytes)
    fn parse_meminfo_line(line: &str) -> Result<(&str, i64)> {
        let (vname, mut vval) = line.split_once(':').context("Splitting meminfo line")?;
        let mut multiplier = 1i64;
        vval = vval.trim();
        vval = match vval.strip_suffix("kB") {
            Some(s) => {
                multiplier *= 1024;
                s.trim_end()
            }
            None => vval,
        };
        vval = match vval.strip_suffix("MB") {
            Some(s) => {
                multiplier *= 1024 * 1024;
                s.trim_end()
            }
            None => vval,
        };
        vval = match vval.strip_suffix("GB") {
            Some(s) => {
                multiplier *= 1024 * 1024 * 1024;
                s.trim_end()
            }
            None => vval,
        };
        vval = match vval.strip_suffix('B') {
            Some(s) => s.trim_end(),
            None => vval,
        };
        let vnum: i64 = vval.parse().context("Parsing meminfo value")?;
        Ok((vname.trim(), vnum * multiplier))
    }

    fn upd_meminfo(&mut self) -> Result<()> {
        if self.fp_meminfo.is_none() {
            self.fp_meminfo = Some(OpenOptions::new().read(true).open("/proc/meminfo")?);
        }
        let fp_meminfo = self.fp_meminfo.as_mut().unwrap();
        self.rdbuf.clear();
        fp_meminfo.read_to_string(&mut self.rdbuf)?;
        fp_meminfo.seek(SeekFrom::Start(0))?;
        for line in self.rdbuf.lines().map(str::trim).filter(|l| !l.is_empty()) {
            let (var, bytes) = Self::parse_meminfo_line(line)?;
            match var {
                "MemTotal" => {
                    self.stats[WhichStat::MemTotal as usize]
                        .value_now
                        .seti(bytes);
                }
                "MemAvailable" => {
                    self.stats[WhichStat::MemAvailable as usize]
                        .value_now
                        .seti(bytes);
                }
                "Active" => {
                    self.stats[WhichStat::MemActive as usize]
                        .value_now
                        .seti(bytes);
                }
                "Inactive" => {
                    self.stats[WhichStat::MemInactive as usize]
                        .value_now
                        .seti(bytes);
                }
                "AnonHugePages" => {
                    self.stats[WhichStat::MemAnonHugePages as usize]
                        .value_now
                        .seti(bytes);
                }
                _ => {}
            }
        }
        Ok(())
    }

    fn upd_net(&mut self) -> Result<()> {
        if self.fp_net.is_none() {
            self.fp_net = Some(OpenOptions::new().read(true).open("/proc/net/dev")?);
            if self.net_iface.is_empty() {
                panic!("Unset network interface to monitor");
            }
            self.net_iface.push(':');
        }
        let fp_net = self.fp_net.as_mut().unwrap();
        self.rdbuf.clear();
        fp_net.read_to_string(&mut self.rdbuf)?;
        fp_net.seek(SeekFrom::Start(0))?;
        let line = self
            .rdbuf
            .lines()
            .map(str::trim)
            .find(|l| l.starts_with(self.net_iface.as_str()))
            .expect("Couldn't find specified network interface");
        let fields = line
            .split_whitespace()
            .map(str::trim)
            .filter(|f| !f.is_empty());
        // Inter-|   Receive                                                |  Transmit
        //  face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
        //    lo: 16445201   19918    0    0    0     0          0         0 16445201   19918    0    0    0     0       0          0
        //f#:  0     1          2     3    4    5     6          7         8 9            10   11    12   13  14     15        16
        let rx: i64 = fields
            .clone()
            .nth(1)
            .expect("Unexpected /proc/net/dev format")
            .parse()?;
        self.stats[WhichStat::NetRxBytes as usize]
            .value_now
            .seti(rx - self.net_rx_ref);
        self.net_rx_ref = rx;

        let rxe: i64 = fields
            .clone()
            .nth(3)
            .expect("Unexpected /proc/net/dev format")
            .parse()?;
        self.stats[WhichStat::NetRxErrors as usize]
            .value_now
            .seti(rxe - self.net_rxe_ref);
        self.net_rxe_ref = rxe;
        let tx: i64 = fields
            .clone()
            .nth(9)
            .expect("Unexpected /proc/net/dev format")
            .parse()?;
        self.stats[WhichStat::NetTxBytes as usize]
            .value_now
            .seti(tx - self.net_tx_ref);
        self.net_tx_ref = tx;
        let txe: i64 = fields
            .clone()
            .nth(10)
            .expect("Unexpected /proc/net/dev format")
            .parse()?;
        self.stats[WhichStat::NetTxErrors as usize]
            .value_now
            .seti(txe - self.net_txe_ref);
        self.net_txe_ref = txe;
        let fcnt = fields.count();
        if fcnt != 17 {
            panic!(
                "Unexpected /proc/net/dev format: got {} fields instead of expected 17",
                fcnt
            );
        }
        Ok(())
    }
}

impl Default for Stats {
    fn default() -> Self {
        Self::new()
    }
}
