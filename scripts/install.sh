#!/usr/bin/env bash
# Install the latest Fenix release into $HOME/fenix and configure the shell.
# Usage:
#   curl -sL https://raw.githubusercontent.com/humbertodias/fenix64/main/scripts/install.sh | bash
set -euo pipefail

REPO="${FENIX_REPO:-humbertodias/fenix64}"
INSTALL_DIR="${FENIX_HOME:-$HOME/fenix}"
VERSION="${FENIX_VERSION:-latest}"
LINKAGE="${FENIX_LINKAGE:-static}"
GITHUB_API="${GITHUB_API:-https://api.github.com}"
GITHUB_DOWNLOAD="${GITHUB_DOWNLOAD:-https://github.com}"

bold() { printf '\033[1m%s\033[0m\n' "$*"; }
info() { printf '  %s\n' "$*"; }
die()  { printf 'ERROR: %s\n' "$*" >&2; exit 1; }

need_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "Required command not found: $1"
}

detect_platform() {
  local os
  os="$(uname -s | tr '[:upper:]' '[:lower:]')"

  case "$os" in
    linux*)  PLATFORM=linux ;;
    darwin*) PLATFORM=macos ;;
    msys*|mingw*|cygwin*) PLATFORM=windows-mingw ;;
    *) die "Unsupported OS: $(uname -s)" ;;
  esac
}

resolve_version() {
  if [[ "$VERSION" != "latest" ]]; then
    TAG="$VERSION"
    return
  fi

  need_cmd curl
  local json
  json="$(curl -fsSL "$GITHUB_API/repos/$REPO/releases/latest")" || \
    die "Failed to query GitHub releases for $REPO"

  TAG="$(printf '%s\n' "$json" | sed -n 's/.*"tag_name"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' | head -n1)"
  [[ -n "$TAG" ]] || die "Could not determine latest release tag"
}

asset_name() {
  local linkage="${1:-}"
  if [[ -n "$linkage" ]]; then
    printf 'fenix-%s-%s-%s.zip' "$PLATFORM" "$linkage" "$TAG"
  else
    printf 'fenix-%s-%s.zip' "$PLATFORM" "$TAG"
  fi
}

find_payload() {
  local stage="$1"
  if [[ -e "$stage/fxc" || -e "$stage/fxc.exe" ]]; then
    printf '%s\n' "$stage"
    return
  fi
  local dir
  dir="$(find "$stage" -mindepth 1 -maxdepth 1 -type d | head -n1)"
  [[ -n "$dir" ]] || die "Archive did not contain fxc / fxc.exe"
  printf '%s\n' "$dir"
}

download_and_extract() {
  local asset url stage extracted
  case "$LINKAGE" in
    static|shared) ;;
    *) die "FENIX_LINKAGE must be static or shared (got: $LINKAGE)" ;;
  esac
  asset="$(asset_name "$LINKAGE")"
  url="$GITHUB_DOWNLOAD/$REPO/releases/download/$TAG/$asset"
  WORK_DIR="$(mktemp -d "${TMPDIR:-/tmp}/fenix-install.XXXXXX")"
  cleanup() { rm -rf "${WORK_DIR:-}"; }
  trap cleanup EXIT

  info "Downloading $asset"
  if ! curl -fL --progress-bar -o "$WORK_DIR/$asset" "$url"; then
    asset="$(asset_name)"
    url="$GITHUB_DOWNLOAD/$REPO/releases/download/$TAG/$asset"
    info "Retrying legacy asset $asset"
    curl -fL --progress-bar -o "$WORK_DIR/$asset" "$url" || \
      die "Download failed (no asset for $PLATFORM/$LINKAGE at $TAG?): $url"
  fi

  stage="$WORK_DIR/extract"
  mkdir -p "$stage"
  if command -v unzip >/dev/null 2>&1; then
    unzip -q "$WORK_DIR/$asset" -d "$stage"
  elif command -v powershell.exe >/dev/null 2>&1; then
    powershell.exe -NoProfile -Command \
      "Expand-Archive -LiteralPath '$WORK_DIR/$asset' -DestinationPath '$stage' -Force"
  else
    die "Need 'unzip' (or powershell.exe) to extract the release zip"
  fi

  extracted="$(find_payload "$stage")"

  rm -rf "$INSTALL_DIR"
  mkdir -p "$INSTALL_DIR"
  cp -a "$extracted"/. "$INSTALL_DIR/"

  if [[ "$PLATFORM" != "windows-mingw" ]]; then
    chmod +x "$INSTALL_DIR/fxc" "$INSTALL_DIR/fxi" "$INSTALL_DIR/map" "$INSTALL_DIR/fpg" 2>/dev/null || true
  fi

  cleanup
  trap - EXIT
}

profile_file() {
  local shell_name
  shell_name="$(basename "${SHELL:-bash}")"
  case "$shell_name" in
    zsh)  printf '%s\n' "${ZDOTDIR:-$HOME}/.zshrc" ;;
    bash)
      if [[ -f "$HOME/.bashrc" ]]; then
        printf '%s\n' "$HOME/.bashrc"
      elif [[ -f "$HOME/.bash_profile" ]]; then
        printf '%s\n' "$HOME/.bash_profile"
      else
        printf '%s\n' "$HOME/.profile"
      fi
      ;;
    fish) printf '%s\n' "$HOME/.config/fish/config.fish" ;;
    *)    printf '%s\n' "$HOME/.profile" ;;
  esac
}

write_env_block() {
  local profile marker_begin marker_end block
  profile="$(profile_file)"
  marker_begin="# >>> fenix >>>"
  marker_end="# <<< fenix <<<"

  mkdir -p "$(dirname "$profile")"
  touch "$profile"

  if [[ "$(basename "${SHELL:-}")" == "fish" ]]; then
    block=$(cat <<EOF
$marker_begin
set -gx FENIX_HOME "$INSTALL_DIR"
if not contains \$FENIX_HOME \$PATH
  set -gx PATH \$FENIX_HOME \$PATH
end
$marker_end
EOF
)
  else
    block=$(cat <<EOF
$marker_begin
export FENIX_HOME="$INSTALL_DIR"
export PATH="\$FENIX_HOME:\$PATH"
$marker_end
EOF
)
  fi

  if grep -qF "$marker_begin" "$profile" 2>/dev/null; then
    local tmp_profile
    tmp_profile="$(mktemp)"
    awk -v begin="$marker_begin" -v end="$marker_end" '
      $0 == begin { skip=1; next }
      $0 == end { skip=0; next }
      !skip { print }
    ' "$profile" > "$tmp_profile"
    printf '\n%s\n' "$block" >> "$tmp_profile"
    mv "$tmp_profile" "$profile"
  else
    printf '\n%s\n' "$block" >> "$profile"
  fi

  PROFILE_UPDATED="$profile"
}

persist_windows_user_env() {
  command -v powershell.exe >/dev/null 2>&1 || return 0

  local win_home
  win_home="$(cygpath -w "$INSTALL_DIR" 2>/dev/null || printf '%s' "$INSTALL_DIR")"

  powershell.exe -NoProfile -Command "
    \$homePath = '$win_home'
    [Environment]::SetEnvironmentVariable('FENIX_HOME', \$homePath, 'User')
    \$path = [Environment]::GetEnvironmentVariable('Path', 'User')
    if (-not \$path) { \$path = '' }
    \$parts = @(\$path -split ';' | Where-Object { \$_ -and \$_ -ne \$homePath })
    \$newPath = (,@(\$homePath) + \$parts) -join ';'
    [Environment]::SetEnvironmentVariable('Path', \$newPath, 'User')
  " >/dev/null || true
}

main() {
  bold "Fenix installer"
  need_cmd curl
  detect_platform
  resolve_version

  info "Version : $TAG"
  info "Platform: $PLATFORM"
  info "Linkage : $LINKAGE"
  info "Install : $INSTALL_DIR"
  echo

  download_and_extract
  write_env_block
  if [[ "$PLATFORM" == "windows-mingw" ]]; then
    persist_windows_user_env
  fi

  export FENIX_HOME="$INSTALL_DIR"
  export PATH="$FENIX_HOME:$PATH"

  echo
  bold "Installed successfully."
  info "FENIX_HOME=$INSTALL_DIR"
  info "Updated shell config: $PROFILE_UPDATED"
  info "Open a new terminal, or run: source \"$PROFILE_UPDATED\""
  info "Then try: fxc -h"
}

main "$@"
