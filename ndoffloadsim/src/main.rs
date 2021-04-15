use std::collections::HashMap;

pub type NodeId = i8;
pub type SimTime = i64;

pub const fn node_mask(n: NodeId) -> i64 {
    1i64 << n
}

pub const NODE_COMPUTE: NodeId = 0;
pub const NODE_STORAGE: NodeId = 1;

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub struct TaskSegment {
    pub time: SimTime,
    pub in_mem: i64,
    pub out_mem: i64,
    pub delta_mem: i64,
    /// Bit-Or of (1 << NODE_ID)
    pub allowed_on: i64,
}

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub struct Task {
    /// Time at which the task is requested to run (it might actually start later in the simulation)
    pub start_time: SimTime,
    pub segments: Vec<TaskSegment>
}

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub struct SchedulingProblem {
    pub tasks: Vec<Task>
}

pub struct SchedulingSolution<'p> {
    pub nodes_allowed: i64,
    pub actual_start_times: HashMap<&'p TaskSegment, (NodeId, SimTime)>,
}



fn main() {
    println!("Hello, world!");
}
