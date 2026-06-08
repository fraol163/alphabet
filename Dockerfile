FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    git \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j$(nproc)

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/alphabet /usr/local/bin/alphabet
COPY --from=builder /app/stdlib /usr/local/share/alphabet/stdlib
COPY --from=builder /app/examples /usr/local/share/alphabet/examples
COPY --from=builder /app/learn /usr/local/share/alphabet/learn
COPY --from=builder /app/docs /usr/local/share/alphabet/docs

ENV ALPHABET_STDLIB_PATH=/usr/local/share/alphabet/stdlib

ENTRYPOINT ["alphabet"]
CMD ["--repl"]
