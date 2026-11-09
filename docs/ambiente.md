# Ambiente de execução

Registro do ambiente usado no desenvolvimento e nas medições do projeto,
conforme a Issue #2.

O ambiente é definido pelo `Dockerfile` na raiz deste repositório, baseado
no ambiente oficial da disciplina. Qualquer pessoa que clone o repositório
e rode `docker compose build` obtém o mesmo conjunto de ferramentas.

---

## Máquina de desenvolvimento

| Item | Valor |
|---|---|
| Processador | AMD Ryzen 9 270 (gráficos integrados Radeon 780M) |
| Arquitetura | x86_64 |
| Núcleos físicos | 8 |
| CPUs lógicas | 16 (2 threads por núcleo) |
| Memória RAM | 15 GiB |
| Cache | L1d 256 KiB, L1i 256 KiB, L2 8 MiB, L3 16 MiB |
| Sistema operacional do hospedeiro | Windows com WSL2 / Docker Desktop |

Foi nesta máquina que o perfilamento e o baseline do Marco 1 foram medidos.

---

## Ferramentas dentro do contêiner

| Item | Valor |
|---|---|
| Imagem base | `ubuntu:24.04` |
| Compilador C++ | g++ 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) |
| Compilador C | gcc, mesma versão |
| Wrappers MPI | `mpic++` e `mpicc` |
| OpenMPI | mpirun (Open MPI) 4.1.6 |
| CPUs visíveis (`nproc`) | 16 |
| Python | python3 com matplotlib e pandas |

A versão do compilador é idêntica à registrada no documento do Marco 1,
confirmando que as medições feitas antes e depois da criação deste ambiente
são comparáveis entre si.

---

## Verificação do ambiente

Versões das ferramentas:

```bash
g++ --version
mpic++ --version
mpirun --version
nproc
python3 -c "import matplotlib, pandas"
```

Compilação e execução do projeto:

```bash
make clean && make
./bin/convolucao_seq 2000 30
```

Compilação e execução dos exemplos fornecidos pelo professor, usados para
confirmar o suporte a OpenMP e a MPI:

```bash
make clean-tudo && make referencia

OMP_NUM_THREADS=4 ./referencia/codigos/01_openmp/01_hello_openmp
mpirun -np 4 ./referencia/codigos/02_mpi/01_hello_mpi
```

| Teste | Resultado |
|---|---|
| Compilação do sequencial com `make` | OK |
| Checksum do sequencial em N=2000, P=30 | 508325215.06 |
| Compilação dos exemplos com `make referencia` | OK |
| Hello world OpenMP com 4 threads | OK — as 4 threads reportaram |
| Hello world MPI com 4 processos | OK — os 4 processos reportaram |

O checksum obtido é idêntico ao registrado no documento do Marco 1, o que
confirma que o ambiente reproduz o resultado de referência.

A ordem em que as threads e os processos imprimem varia a cada execução.
Isso é esperado: não há sincronização entre eles na saída, e cada um
escreve assim que chega ao `printf`.

---

## Observações

- A tag `ubuntu:24.04` é móvel: o `apt` instala o que estiver atual no dia
  em que a imagem é construída. As versões acima são as obtidas na
  construção desta imagem e devem ser reconferidas se a imagem for
  reconstruída muito tempo depois.
- Dentro do contêiner o `mpirun` roda como root sem flag adicional, porque
  o `Dockerfile` define `OMPI_ALLOW_RUN_AS_ROOT`. No laboratório, onde o
  usuário não é root, essas variáveis são ignoradas e nada muda.
- Se o número de processos passado ao `mpirun` for maior que o número de
  CPUs disponíveis, é preciso acrescentar `--oversubscribe`.
- Os tempos impressos nesta verificação não são medições: são execuções
  únicas, sem descarte de aquecimento nem repetição. As medições oficiais
  seguem o protocolo definido na Issue #7.
- Tempos medidos em máquinas diferentes não são comparáveis entre si. Cada
  curva de speedup precisa ser construída com todas as medições feitas na
  mesma máquina.
- O número de CPUs lógicas não é o número de núcleos reais. Com 8 núcleos
  físicos e 16 CPUs lógicas, o speedup de um trabalho limitado por CPU
  tende a saturar perto de 8, e não de 16 — as duas threads de um mesmo
  núcleo disputam as mesmas unidades de execução.

---