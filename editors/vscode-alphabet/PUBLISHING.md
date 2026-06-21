# Publishing to the VS Code Marketplace

This guide walks through publishing the Alphabet extension so users can install it like any other VS Code extension.

## One-time setup

### 1. Create a Microsoft / Azure DevOps publisher account

1. Sign in at https://aka.ms/vscode-create-publisher with your Microsoft account
2. Choose a unique **Publisher ID** (we use `fraolteshome` in `package.json`)
3. Optionally fill in your display name and profile

### 2. Generate a Personal Access Token (PAT)

1. Go to https://dev.azure.com → top-right **User settings** → **Personal access tokens**
2. Click **+ New Token**
3. Settings:
   - **Name**: `vscode-marketplace`
   - **Organization**: All accessible organizations
   - **Expiration**: 90 days (or your security policy)
   - **Scopes**: Custom → check **Marketplace** → **Manage**
4. Click **Create** and **copy the token immediately** — it's shown only once

### 3. Store the token locally

Add to your shell config (or use a secret manager):

```bash
# ~/.bashrc or ~/.zshrc
export VSCE_PAT="<paste-your-token-here>"
```

Or for OpenVSX (optional, for Cursor/VSCodium):

```bash
export OVSX_PAT="<your-openvsx-token>"
```

Get an OpenVSX token at https://open-vsx.org → sign in → **Settings** → **Access Tokens**.

## Publishing

### Option A: Local publish (fast, manual)

```bash
cd editors/vscode-alphabet
npm run publish          # bumps version + publishes to Marketplace
```

Or, to publish a pre-built VSIX without rebuilding:

```bash
npx @vscode/vsce publish --packagePath alphabet-0.1.0.vsix
```

### Option B: GitHub Actions release (recommended, automated)

Already configured in `.github/workflows/vscode-extension-release.yml`.

**Setup once:**

1. Add `VSCE_PAT` and `OVSX_PAT` as GitHub repository secrets:
   - Repo → **Settings** → **Secrets and variables** → **Actions** → **New repository secret**
2. Tag a release:
   ```bash
   git tag v0.1.0
   git push origin v0.1.0
   ```
3. The workflow:
   - Builds the C++ `alphabet` binary on linux/mac/windows (x64 + arm64)
   - Stages binaries into `server/bin/`
   - Bundles TypeScript
   - Packages VSIX
   - Publishes to VS Code Marketplace
   - Publishes to OpenVSX
   - Creates GitHub Release with all artifacts attached

**Test the workflow without publishing** (manual trigger):

```bash
# Actions tab → "Release VS Code Extension" → Run workflow
# Set version input to e.g. 0.1.0
# This builds + packages but skips publish jobs (no tag)
```

## Marketplace listing checklist

Before your first publish, polish the Marketplace listing:

- [ ] Icon uploaded (128×128 PNG, square, no transparency)
- [ ] Display name: **Alphabet Language**
- [ ] Description: matches what's in `package.json`
- [ ] Category: Programming Languages
- [ ] Tags: `alphabet`, `multilingual`, `lsp`, `amharic`, `wasm`
- [ ] Repository URL: links to your GitHub
- [ ] Homepage: links to your docs site
- [ ] License: MIT
- [ ] Screenshots (optional but recommended):
  - Syntax highlighting in action (en + am)
  - Completion dropdown
  - Hover docs
  - NL-to-Code panel

Update Marketplace metadata at: https://marketplace.visualstudio.com/manage → click your publisher → edit extension.

## Versioning

We follow [Semantic Versioning](https://semver.org/):

- **Patch** (0.1.x): Bug fixes, no LSP protocol changes
- **Minor** (0.x.0): New features, backward-compatible
- **Major** (x.0.0): Breaking LSP changes or major rewrites

Bump version in **three** places (the workflow handles all three on tag push):
- `editors/vscode-alphabet/package.json` → `"version"`
- Root `VERSION` (used by C++ binary)
- Tag like `v0.1.0`

## Updating the bundled binary

The bundled `alphabet-linux-x64` (and friends) lives in `editors/vscode-alphabet/server/bin/`.

**To update locally:**

```bash
cd /path/to/Alphabet_Language
cmake --build build --target alphabet --config Release
cp build/alphabet editors/vscode-alphabet/server/bin/alphabet-linux-x64
```

**In CI:** the release workflow rebuilds binaries from source per platform.

## Pre-release channel

To publish a beta channel:

1. In `package.json`, add `"prerelease": true` and bump to e.g. `0.2.0-beta.1`
2. Tag: `git tag v0.2.0-beta.1 && git push origin v0.2.0-beta.1`
3. The workflow publishes as pre-release; users opt-in via the **Install Pre-Release** button in Marketplace

## Uninstall metrics

You can monitor adoption in the Marketplace publisher dashboard:
- Total installs
- Active installs (last 30 days)
- Uninstall rate
- Average rating
- Review sentiment

Use these to decide when to ship a breaking change or when to focus on stability vs features.

## Troubleshooting publish

| Error | Fix |
|---|---|
| `Forbidden: Publisher not found` | Wrong publisher ID in `package.json` |
| `Personal Access Token expired` | Regenerate PAT in Azure DevOps |
| `Extension already exists` | Bump version in `package.json` |
| `Invalid icon` | PNG must be 128×128, no transparency |
| `Missing LICENSE` | Ensure `LICENSE`, `LICENSE.md`, or `LICENSE.txt` exists at root |

## Reference

- VS Code Marketplace: https://marketplace.visualstudio.com
- Publishing docs: https://code.visualstudio.com/api/working-with-extensions/publishing-extension
- vsce CLI: https://github.com/microsoft/vscode-vsce
- OpenVSX: https://open-vsx.org
