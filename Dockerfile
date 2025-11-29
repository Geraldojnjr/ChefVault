# Stage 1: Build
FROM ubuntu:22.04 AS builder

# Evitar prompts interativos durante instalação
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependências de build
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

# Copiar código fonte
WORKDIR /app
COPY . .

# Criar diretório de build
RUN mkdir -p build && cd build && \
    cmake .. && \
    cmake --build . && \
    cmake --install . --prefix /app/install

# Stage 2: Runtime
FROM ubuntu:22.04

# Instalar apenas runtime dependencies
RUN apt-get update && apt-get install -y \
    libsqlite3-0 \
    && rm -rf /var/lib/apt/lists/*

# Copiar binário do stage de build
WORKDIR /app
COPY --from=builder /app/build/cookbook /app/cookbook

# Criar diretório para dados
RUN mkdir -p /app/data

# Tornar executável
RUN chmod +x /app/cookbook

# Comando padrão
CMD ["./cookbook"]

