#!/usr/bin/env bash
# Run the matrix program multiple times and collect results
current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
source ${current_dir}/set_env.sh

export MODELS_DIR=${PROJECT_BIN_DIR}/models
mkdir -p ${PROJECT_OUT_DIR}

run() {
  suffix=$(echo $options | sed 's/[ -]//g')
  outfile="${PROJECT_OUT_DIR}/output${suffix}.ppm"
  if [ -f $outfile ]; then
    echo "Deleting existing output at $outfile"
    rm $outfile
  fi
  echo "${PROJECT_BIN_DIR}/${program} -m '${MODELS_DIR}/${model}' -v '${eye}' -l '${look}' ${options} -o ${outfile}"
  "${PROJECT_BIN_DIR}/${program}" -m "${MODELS_DIR}/${model}" -v "${eye}" -l "${look}" -o "${outfile}" ${options}
  if [ -f $outfile ]; then
    echo "Created new rendering at $outfile"
    echo "Opening in default viewer"
    open $outfile
  else
    echo "WARNING: expected file to be created, but could find none at ${outfile}"
  fi
}

model="cornell/CornellBox-Original.obj"
eye="0,1,2"
look="0,1,0"
program="ESCViewer2021"
# Run threaded
#options="--thread"
# Flatten triangles from scene
for options in {"--flat","--flat --thread"}
do
  run
done


