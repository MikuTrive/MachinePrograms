#!/usr/bin/env bash
set -u

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
LANG_FILE="$SCRIPT_DIR/.msh_lang"
MACHINE_BIN="$SCRIPT_DIR/machine"
EXAMPLES_DIR="$SCRIPT_DIR/test"

TEST_NAMES=(
  "struct"
  "while"
  "while-for"
  "const"
  "me"
  "dyna"
  "list"
  "mod"
  "elif"
)

C_RESET='\033[0m'
C_WHITE='\033[1;37m'
C_GREEN='\033[1;32m'
C_RED='\033[1;31m'
C_CYAN='\033[1;36m'
C_YELLOW='\033[1;33m'
C_REVERSE='\033[7m'

current_lang() {
  if [[ -f "$LANG_FILE" ]]; then
    local saved
    saved="$(tr -d ' \t\r\n' < "$LANG_FILE")"
    if [[ "$saved" == "zh_CN" || "$saved" == "en_US" ]]; then
      printf '%s' "$saved"
      return
    fi
  fi
  printf '%s' "en_US"
}

lang() {
  current_lang
}

msg() {
  local key="$1"
  local l
  l="$(lang)"

  if [[ "$l" == "zh_CN" ]]; then
    case "$key" in
      help_title) printf '%s\n' 'Machine language test helper' ;;
      help_usage) printf '%s\n' 'Usage: ./m.sh [option]' ;;
      help_none) printf '%s\n' 'No option: show this help message.' ;;
      help_c) printf '%s\n' '  -c    Compile and run all predefined Machine test programs.' ;;
      help_d) printf '%s\n' '  -d    Delete compiled test binaries.' ;;
      help_l) printf '%s\n' '  -l    Choose output language (zh_CN / en_US).' ;;
      help_h) printf '%s\n' '  -h    Show this help message.' ;;
      missing_machine) printf '%s\n' 'Machine compiler not found: ./machine' ;;
      missing_examples) printf '%s\n' 'Examples directory not found: ./examples' ;;
      select_title) printf '%s\n' 'Use Up/Down arrow keys, then press Enter to confirm:' ;;
      selected_lang) printf '%s\n' 'Language saved.' ;;
      cleanup_done) printf '%s\n' 'Compiled test binaries removed.' ;;
      cleanup_none) printf '%s\n' 'No compiled test binaries found.' ;;
      compile_output) printf '%s\n' 'Compiler output:' ;;
      run_output) printf '%s\n' 'Program output:' ;;
      source_missing) printf '%s' '源文件缺失' ;;
      compile_failed_suffix) printf '%s' '编译失败，请核对代码；如代码无误请检查 Machine 项目本身！' ;;
      compile_ok_suffix) printf '%s' '编译通过并运行完成，请核对结果是否正确。' ;;
      run_failed_suffix) printf '%s' '编译通过但运行失败，请检查输出与运行环境。' ;;
      deleting) printf '%s' 'Deleting' ;;
      testing) printf '%s' 'Testing' ;;
      *) printf '%s\n' "$key" ;;
    esac
  else
    case "$key" in
      help_title) printf '%s\n' 'Machine language test helper' ;;
      help_usage) printf '%s\n' 'Usage: ./m.sh [option]' ;;
      help_none) printf '%s\n' 'No option: show this help message.' ;;
      help_c) printf '%s\n' '  -c    Compile and run all predefined Machine test programs.' ;;
      help_d) printf '%s\n' '  -d    Delete compiled test binaries.' ;;
      help_l) printf '%s\n' '  -l    Choose output language (zh_CN / en_US).' ;;
      help_h) printf '%s\n' '  -h    Show this help message.' ;;
      missing_machine) printf '%s\n' 'Machine compiler not found: ./machine' ;;
      missing_examples) printf '%s\n' 'Examples directory not found: ./examples' ;;
      select_title) printf '%s\n' 'Use Up/Down arrow keys, then press Enter to confirm:' ;;
      selected_lang) printf '%s\n' 'Language saved.' ;;
      cleanup_done) printf '%s\n' 'Compiled test binaries removed.' ;;
      cleanup_none) printf '%s\n' 'No compiled test binaries found.' ;;
      compile_output) printf '%s\n' 'Compiler output:' ;;
      run_output) printf '%s\n' 'Program output:' ;;
      source_missing) printf '%s' 'source file missing' ;;
      compile_failed_suffix) printf '%s' 'compile failed, please check the code; if the code is correct, inspect the Machine project itself!' ;;
      compile_ok_suffix) printf '%s' 'compiled and ran successfully, please verify whether the output is correct.' ;;
      run_failed_suffix) printf '%s' 'compiled successfully but failed at runtime, please inspect the output and runtime environment.' ;;
      deleting) printf '%s' 'Deleting' ;;
      testing) printf '%s' 'Testing' ;;
      *) printf '%s\n' "$key" ;;
    esac
  fi
}

status_ok_prefix() {
  printf '%b[%bOK%b]%b' "$C_WHITE" "$C_GREEN" "$C_WHITE" "$C_RESET"
}

status_failed_prefix() {
  printf '%b[%bFAILED%b]%b' "$C_WHITE" "$C_RED" "$C_WHITE" "$C_RESET"
}

print_help() {
  msg help_title
  msg help_usage
  echo
  msg help_none
  msg help_c
  msg help_d
  msg help_l
  msg help_h
}

ensure_layout() {
  if [[ ! -x "$MACHINE_BIN" ]]; then
    msg missing_machine
    exit 1
  fi
  if [[ ! -d "$EXAMPLES_DIR" ]]; then
    msg missing_examples
    exit 1
  fi
}

compile_and_run_one() {
  local name="$1"
  local src="$EXAMPLES_DIR/$name.mne"
  local bin="$SCRIPT_DIR/$name"
  local compile_log
  local run_log
  compile_log="$(mktemp)"
  run_log="$(mktemp)"

  echo
  printf '%b== %s: %s ==%b\n' "$C_CYAN" "$(msg testing)" "$name" "$C_RESET"

  if [[ ! -f "$src" ]]; then
    status_failed_prefix
    printf ' %s: %s\n' "$name" "$(msg source_missing)"
    rm -f "$compile_log" "$run_log"
    return
  fi

  if "$MACHINE_BIN" "$src" -o "$bin" >"$compile_log" 2>&1; then
    if [[ -s "$compile_log" ]]; then
      msg compile_output
      cat "$compile_log"
    fi

    if "$bin" >"$run_log" 2>&1; then
      msg run_output
      cat "$run_log"
      status_ok_prefix
      printf ' %s %s\n' "$name" "$(msg compile_ok_suffix)"
    else
      msg run_output
      cat "$run_log"
      status_failed_prefix
      printf ' %s %s\n' "$name" "$(msg run_failed_suffix)"
    fi
  else
    msg compile_output
    cat "$compile_log"
    status_failed_prefix
    printf ' %s %s\n' "$name" "$(msg compile_failed_suffix)"
  fi

  rm -f "$compile_log" "$run_log"
}

run_all_tests() {
  ensure_layout
  local name
  for name in "${TEST_NAMES[@]}"; do
    compile_and_run_one "$name"
  done
}

delete_binaries() {
  ensure_layout
  local removed=0
  local name
  for name in "${TEST_NAMES[@]}"; do
    if [[ -e "$SCRIPT_DIR/$name" ]]; then
      rm -f -- "$SCRIPT_DIR/$name"
      removed=1
    fi
  done

  if [[ $removed -eq 1 ]]; then
    msg cleanup_done
  else
    msg cleanup_none
  fi
}

choose_language() {
  local options=("zh_CN" "en_US")
  local selected=0
  local key

  while true; do
    clear
    msg select_title
    echo

    local i=0
    while [[ $i -lt ${#options[@]} ]]; do
      if [[ $i -eq $selected ]]; then
        printf '%b> %s%b\n' "$C_REVERSE" "${options[$i]}" "$C_RESET"
      else
        printf '  %s\n' "${options[$i]}"
      fi
      i=$((i + 1))
    done

    IFS= read -rsn1 key
    if [[ "$key" == $'\x1b' ]]; then
      IFS= read -rsn2 key
      case "$key" in
        '[A')
          if [[ $selected -gt 0 ]]; then
            selected=$((selected - 1))
          fi
          ;;
        '[B')
          if [[ $selected -lt $((${#options[@]} - 1)) ]]; then
            selected=$((selected + 1))
          fi
          ;;
      esac
    elif [[ "$key" == "" ]]; then
      printf '%s\n' "${options[$selected]}" > "$LANG_FILE"
      clear
      msg selected_lang
      break
    fi
  done
}

main() {
  case "${1:-}" in
    '')
      print_help
      ;;
    '-c')
      run_all_tests
      ;;
    '-d')
      delete_binaries
      ;;
    '-l')
      choose_language
      ;;
    '-h'|'--help')
      print_help
      ;;
    *)
      print_help
      exit 1
      ;;
  esac
}

main "$@"
