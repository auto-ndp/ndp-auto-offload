use std::collections::HashMap;
use std::fmt::Write as _;
use std::io::prelude::*;
use std::{fs, io, path::*};

#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Debug, Default, Hash)]
pub struct PageNumber(pub u64);

#[derive(Clone, Copy, PartialEq, Eq, Debug, Hash)]
pub enum AccessType {
    Read,
    Write,
    ReadWrite,
}

impl AccessType {
    pub fn try_from_rw(is_read: bool, is_write: bool) -> Option<Self> {
        match (is_read, is_write) {
            (true, false) => Some(Self::Read),
            (false, true) => Some(Self::Write),
            (true, true) => Some(Self::ReadWrite),
            _ => None,
        }
    }

    pub fn is_read(&self) -> bool {
        return matches!(self, Self::Read | Self::ReadWrite);
    }

    pub fn is_write(&self) -> bool {
        return matches!(self, Self::Write | Self::ReadWrite);
    }
}

#[derive(Clone, Debug)]
pub struct TracePhase {
    pub name: String,
    pub page_size: u64,
    pub start_time: u64,
    pub instructions: u64,
    pub accesses: HashMap<PageNumber, AccessType>,
}

impl TracePhase {
    fn new(name: String, start_time: u64, instructions: u64) -> Self {
        Self {
            name,
            start_time,
            instructions,
            accesses: HashMap::with_capacity(1024 * 1024),
        }
    }
}

fn analyse_file(path: &Path, out: &mut String) -> io::Result<()> {
    let f = fs::File::open(path)?;
    let reader = io::BufReader::new(f);
    let mut phases: Vec<TracePhase> = Vec::with_capacity(8);
    let mut next_phase = "pre-main".to_owned();
    for line in reader.lines() {
        let line = line?;
        if line.starts_with("trace;") {
            let phase = phases.last_mut().unwrap();
            // trace;;0;56962;2190968395311;RW;
            let mut spl = line.split(';');
            let _ = spl.next().unwrap(); // trace
            let _ = spl.next().unwrap(); // ;;
            let _ = spl.next().unwrap(); // start time
            let _ = spl.next().unwrap(); // end time
            let pageidx = PageNumber(spl.next().unwrap().parse().unwrap());
            let accessty = spl.next().unwrap();
            let accessty =
                AccessType::try_from_rw(accessty.contains('R'), accessty.contains('W')).unwrap();
            phase.accesses.insert(pageidx, accessty);
        } else if line.starts_with("phase;") {
            // phase;0;56962;main-start
            let mut spl = line.split(';');
            let _ = spl.next().unwrap();
            let page_size: u64 = spl.next().unwrap().parse().unwrap();
            let prev_start_time: u64 = spl.next().unwrap().parse().unwrap();
            let _prev_end_time: u64 = spl.next().unwrap().parse().unwrap();
            let name: String = spl.next().unwrap().to_owned();
            let instructions: u64 = spl.next().unwrap_or("0").parse().unwrap();
            phases.push(TracePhase::new(
                std::mem::replace(&mut next_phase, name),
                page_size,
                prev_start_time,
                instructions,
            ));
        }
    }
    for (i, phase) in phases.iter().enumerate() {
        let mut read_pages: u64 = 0;
        let mut written_pages: u64 = 0;
        let mut dead_writes: u64 = 0;
        let mut new_reads: u64 = 0;
        for (pid, access) in phase.accesses.iter() {
            if access.is_read() {
                read_pages += 1;
                if phases[..i].iter().all(|p| p.accesses.get(pid).is_none()) {
                    new_reads += 1;
                }
            }
            if access.is_write() {
                written_pages += 1;
                if phases[i + 1..]
                    .iter()
                    .all(|p| p.accesses.get(pid).map(|at| !at.is_read()).unwrap_or(true))
                {
                    dead_writes += 1;
                }
            }
        }
        writeln!(
            out,
            "{},{},{},{},{},{},{}",
            path.display(),
            phase.name,
            read_pages,
            written_pages,
            dead_writes,
            new_reads,
            instructions,
        )
        .unwrap();
    }
    Ok(())
}

/// Outputs CSV
fn analyse_folder(folder: &Path, out: &mut String) -> io::Result<()> {
    writeln!(out, "{}", folder.display()).unwrap();
    out.push_str("Trace,Phase,Page size,Read pages,Written pages,Dead written pages,New reads,Instructions\n");
    for file in fs::read_dir(folder)? {
        let file = file?;
        let path = file.path();
        if path.extension().map(|e| e != "log").unwrap_or(true) {
            continue;
        }
        eprintln!("Analysing {:?}", path);
        analyse_file(&path, out)?;
    }
    Ok(())
}

fn main() -> io::Result<()> {
    let args = std::env::args_os().skip(1);
    let mut out = String::with_capacity(4096);
    for path in args {
        analyse_folder(&PathBuf::from(path), &mut out)?;
    }
    fs::write("memory_access_summary.csv", &out)?;
    Ok(())
}
