# Ambiente de execução do projeto Filtro de Convolução 2D (blur 3x3)
# CCO085 — IESB 2026/2
# Baseado no ambiente oficial da disciplina (Prof. Rodrigo Gonçalves Pinto)
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV OMPI_ALLOW_RUN_AS_ROOT=1
ENV OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        gdb \
        valgrind \
        libomp-dev \
        openmpi-bin \
        libopenmpi-dev \
        python3 \
        python3-pip \
        python3-matplotlib \
        python3-pandas \
        time \
        nano \
        less \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /lab
CMD ["/bin/bash"]