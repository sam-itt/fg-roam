#!/usr/bin/env bash

set -Eeuo pipefail
trap cleanup SIGINT SIGTERM ERR EXIT

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)

usage() {
  cat <<EOF
Usage: $(basename "${BASH_SOURCE[0]}") [-h] [-v] [-f fg-data-root] -d texroot textures.txt

Copy textures listed in textures.txt (one per line) from flightgear data dir
and save them in texroot dir.

Available options:

-h, --help                 Print this help and exit
-v, --verbose              Print script debug info
-d, --output-directory     Use dir as texture root
-f, --fg-data-root         FG_DATA_ROOT, defaults to /usr/share/flightgear
EOF
  exit
}

cleanup() {
  trap - SIGINT SIGTERM ERR EXIT
  # script cleanup here
}

setup_colors() {
  if [[ -t 2 ]] && [[ -z "${NO_COLOR-}" ]] && [[ "${TERM-}" != "dumb" ]]; then
    NOFORMAT='\033[0m' RED='\033[0;31m' GREEN='\033[0;32m' ORANGE='\033[0;33m' BLUE='\033[0;34m' PURPLE='\033[0;35m' CYAN='\033[0;36m' YELLOW='\033[1;33m'
  else
    NOFORMAT='' RED='' GREEN='' ORANGE='' BLUE='' PURPLE='' CYAN='' YELLOW=''
  fi
}

msg() {
  echo >&2 -e "${1-}"
}

msgn() {
  echo >&2 -ne "${1-}"
}


die() {
  local msg=$1
  local code=${2-1} # default exit status 1
  msg "$msg"
  exit "$code"
}

parse_params() {
  texroot=''
  fgdataroot='/usr/share/flightgear'

  while :; do
    case "${1-}" in
    -h | --help) usage ;;
    -v | --verbose) set -x ;;
    --no-color) NO_COLOR=1 ;;
    -d | --output-directory)
      texroot="${2-}"
      shift
      ;;
    -f | --fg-root)
      fgdataroot="${2-}"
      shift
      ;;
    -?*) die "Unknown option: $1" ;;
    *) break ;;
    esac
    shift
  done

  args=("$@")

  # check required params and arguments
  [[ -z "${texroot-}" ]] && die "Missing required parameter: texroot"
  [[ ${#args[@]} -eq 0 ]] && die "Missing script arguments"

  return 0
}

parse_params "$@"
setup_colors

msgn "Copying files ..."
mkdir -p "${texroot}/full"
while IFS= read -r line; do
    rel_path=$(dirname $line)
    mkdir -p "${texroot}/full/${rel_path}"
    cp "${fgdataroot}/Textures/${line}" "${texroot}/full/${rel_path}"
done < ${args[*]-}
msgn "\t[ ${GREEN}DONE${NOFORMAT} ]\n"

msgn "Converting all files to RGBA ..."
find "${texroot}/full" -type f -exec mogrify -define png:format=png32 -format png {} \;
msgn "\t[ ${GREEN}DONE${NOFORMAT} ]\n"

msgn "Creating smaller versions ..."
cp -r "${texroot}/full" "${texroot}/small"
find "${texroot}/small" -not -path "*/Runway/*" -type f -exec mogrify -define png:format=png32 -format png -resize 1x1 {} \;
msgn "\t[ ${GREEN}DONE${NOFORMAT} ]\n"

