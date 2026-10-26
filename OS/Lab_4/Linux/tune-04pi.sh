#!/bin/bash
EXE=${1:-./Lab-04pi}
PART=${2:-2000}
SECONDS_PER_RUN=${3:-2}
fibers=(1 4 16 64)
slices=(1 10 100 1000 10000 100000)

echo "Throughput, Mterms/s (partial sum size = $PART, $SECONDS_PER_RUN s per run)"
printf "%-8s" "Fibers"
for q in "${slices[@]}"; do
    printf "%-14s" "slice=$q"
done
printf "\n"
for f in "${fibers[@]}"; do
    printf "%-8s" "$f"
    for q in "${slices[@]}"; do
        rate=$("$EXE" "$f" "$PART" "$q" "$SECONDS_PER_RUN" | sed -n 's/^Rate: \([0-9.]*\) Mterms\/s$/\1/p')
        printf "%-14s" "${rate:-error}"
    done
    printf "\n"
done
