FROM rust:1.55-slim-buster as builder

WORKDIR /usr/src/statmon

COPY . .
RUN touch src/main.rs
RUN cargo build --release
RUN strip target/release/statmon

FROM debian:buster-slim
COPY --from=builder /usr/src/statmon/target/release/statmon /usr/local/bin/statmon
ENV STATMON_NETWORK_DEVICE=lo
ENV STATMON_PORT=8125
ENV STATMON_HOST_PREFIX=local
ENTRYPOINT "statmon" "--netdev" "${STATMON_NETWORK_DEVICE}" "--port" "${STATMON_PORT}" "--host-prefix" "${STATMON_HOST_PREFIX}"