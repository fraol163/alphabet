import {
  ExtensionContext,
  WebviewViewProvider,
  WebviewView,
  WebviewViewResolveContext,
  CancellationToken,
  OutputChannel,
  commands,
  workspace,
} from 'vscode';
import { Uri, Webview } from 'vscode';
import { execFile } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';

export class NLToCodeProvider implements WebviewViewProvider {
  public static readonly viewType = 'alphabet.nlToCode';

  constructor(
    private readonly context: ExtensionContext,
    private readonly output: OutputChannel,
  ) {}

  resolveWebviewView(
    webviewView: WebviewView,
    _context: WebviewViewResolveContext,
    _token: CancellationToken,
  ): void | Thenable<void> {
    webviewView.webview.options = {
      enableScripts: true,
      localResourceRoots: [Uri.joinPath(this.context.extensionUri, 'resources')],
    };
    webviewView.webview.html = this.getHtml(webviewView.webview);
    webviewView.webview.onDidReceiveMessage(async (msg) => {
      if (msg.type === 'requestCode') {
        this.output.appendLine(`[nl-to-code] request: ${msg.text}`);
        try {
          const code = await this.generateCode(msg.text);
          webviewView.webview.postMessage({ type: 'codeResult', code, source: code.startsWith('// [placeholder]') ? 'placeholder' : 'binary' });
        } catch (err: any) {
          this.output.appendLine(`[nl-to-code] error: ${err?.message ?? String(err)}`);
          const fallback = this.placeholderCode(msg.text);
          webviewView.webview.postMessage({
            type: 'codeResult',
            code: fallback,
            source: 'placeholder',
            warning: `alphabet binary failed: ${err?.message ?? String(err)}`,
          });
        }
      }
    });
  }

  open(): void {
    void commands.executeCommand('alphabet.nlToCode.focus');
  }

  /**
   * Call `alphabet nl "<prompt>"` if the binary is on PATH or bundled.
   * Falls back to a deterministic stub if the CLI doesn't expose `nl`.
   */
  private generateCode(prompt: string): Promise<string> {
    return new Promise((resolve, reject) => {
      const binary = this.findBinary();
      if (!binary) {
        resolve(this.placeholderCode(prompt));
        return;
      }
      execFile(binary, ['nl', prompt], { timeout: 15000 }, (err, stdout, stderr) => {
        if (err) {
          // Most binaries don't implement `nl` yet — fall back silently
          // to the placeholder rather than failing the user.
          if (/unrecognized|unknown|invalid/i.test(stderr || '') ||
              (err as any).code === 'ENOENT' ||
              (err as any).signal) {
            resolve(this.placeholderCode(prompt));
          } else {
            // Treat non-zero exit as "feature unavailable" too.
            resolve(this.placeholderCode(prompt));
          }
          return;
        }
        const trimmed = (stdout || '').trim();
        if (trimmed.length > 0) {
          resolve(trimmed + '\n');
        } else {
          resolve(this.placeholderCode(prompt));
        }
      });
    });
  }

  private findBinary(): string | undefined {
    // 1. Configured override
    const configured = workspace
      .getConfiguration('alphabet.lsp')
      .get<string>('path');
    if (configured && configured.trim().length > 0) return configured;

    // 2. Bundled
    const platform = process.platform;
    const arch = process.arch;
    const isWin = platform === 'win32';
    const ext = isWin ? '.exe' : '';
    const platformName =
      platform === 'darwin' ? 'darwin' :
      platform === 'win32'  ? 'win32'  :
                               'linux';
    const archName = arch === 'arm64' ? 'arm64' : 'x64';
    const bundled = `server/bin/alphabet-${platformName}-${archName}${ext}`;
    const bundledPath = path.join(this.context.extensionPath, bundled);
    if (fs.existsSync(bundledPath)) return bundledPath;

    // 3. PATH
    const PATH = process.env.PATH ?? '';
    const sep = path.delimiter;
    const exts = isWin
      ? (process.env.PATHEXT ?? '.EXE;.CMD;.BAT').split(';')
      : [''];
    for (const dir of PATH.split(sep)) {
      if (!dir) continue;
      for (const e of exts) {
        const candidate = path.join(dir, 'alphabet' + e);
        if (fs.existsSync(candidate)) return candidate;
      }
    }
    return undefined;
  }

  private placeholderCode(prompt: string): string {
    // Deterministic stub used when the binary or `alphabet nl` subcommand is
    // unavailable. Marks itself as a placeholder so the webview can warn.
    const lines = prompt.trim().split(/\s+/);
    const varName = (lines[0] ?? 'x').toLowerCase().replace(/[^a-z0-9_]/g, '') || 'x';
    return [
      '// [placeholder] Generated from prompt: "' + prompt + '"',
      '#alphabet<en>',
      '',
      `5 ${varName} = 0`,
      '',
      'm 0 main() {',
      `  z.o(${varName})`,
      '}',
      '',
    ].join('\n');
  }

  private getHtml(webview: Webview): string {
    const nonce = String(Math.random()).slice(2);
    const csp = `default-src 'none'; style-src ${webview.cspSource} 'unsafe-inline'; script-src 'nonce-${nonce}';`;
    return `<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta http-equiv="Content-Security-Policy" content="${csp}" />
<style>
  :root {
    color-scheme: light dark;
    --bg: var(--vscode-editor-background);
    --fg: var(--vscode-editor-foreground);
    --accent: var(--vscode-button-background);
    --accent-fg: var(--vscode-button-foreground);
    --border: var(--vscode-panel-border);
    --warn-bg: var(--vscode-inputValidation-warningBackground);
  }
  body { margin: 0; padding: 12px; font-family: var(--vscode-font-family); background: var(--bg); color: var(--fg); }
  h3 { margin: 0 0 8px 0; font-size: 13px; font-weight: 600; }
  p { font-size: 12px; opacity: 0.7; margin: 0 0 12px 0; }
  textarea {
    width: 100%; height: 80px; box-sizing: border-box;
    background: var(--vscode-input-background); color: var(--vscode-input-foreground);
    border: 1px solid var(--border); border-radius: 4px; padding: 6px 8px; resize: vertical;
    font-family: var(--vscode-font-family); font-size: 12px;
  }
  button {
    margin-top: 8px; padding: 6px 12px;
    background: var(--accent); color: var(--accent-fg); border: none; border-radius: 4px;
    cursor: pointer; font-size: 12px;
  }
  button:hover { filter: brightness(1.1); }
  button:disabled { opacity: 0.5; cursor: wait; }
  pre {
    margin-top: 12px; padding: 8px; background: var(--vscode-textCodeBlock-background);
    border: 1px solid var(--border); border-radius: 4px;
    font-family: var(--vscode-editor-font-family); font-size: 12px; white-space: pre-wrap;
  }
  .meta { display: flex; gap: 6px; align-items: center; margin-top: 10px; font-size: 11px; opacity: 0.85; }
  .badge {
    display: inline-block; padding: 2px 6px; border-radius: 8px;
    background: var(--vscode-badge-background); color: var(--vscode-badge-foreground);
    font-size: 10px; font-weight: 600; letter-spacing: 0.3px;
  }
  .badge.placeholder { background: var(--warn-bg); }
  .warn { margin-top: 6px; font-size: 11px; opacity: 0.85; }
</style>
</head>
<body>
<h3>NL → Alphabet</h3>
<p>Describe what you want. Get Alphabet code.</p>
<textarea id="prompt" placeholder="e.g. read a number and print its square"></textarea>
<button id="generate">Generate</button>
<pre id="result" hidden></pre>
<div class="meta">
  <span id="source" class="badge" hidden></span>
  <button id="copy" hidden style="margin:0;padding:2px 8px;font-size:11px">Copy</button>
</div>
<div id="warn" class="warn" hidden></div>
<script nonce="${nonce}">
  const vscode = acquireVsCodeApi();
  const ta = document.getElementById('prompt');
  const btn = document.getElementById('generate');
  const copyBtn = document.getElementById('copy');
  const out = document.getElementById('result');
  const src = document.getElementById('source');
  const warn = document.getElementById('warn');
  btn.addEventListener('click', () => {
    const text = ta.value.trim();
    if (!text) return;
    btn.disabled = true; btn.textContent = 'Generating...';
    vscode.postMessage({ type: 'requestCode', text });
  });
  copyBtn.addEventListener('click', async () => {
    try {
      await navigator.clipboard.writeText(out.textContent || '');
      copyBtn.textContent = 'Copied!';
      setTimeout(() => { copyBtn.textContent = 'Copy'; }, 1200);
    } catch (_) {}
  });
  window.addEventListener('message', (e) => {
    const msg = e.data;
    if (msg.type === 'codeResult') {
      out.hidden = false;
      out.textContent = msg.code;
      copyBtn.hidden = false;
      const isPlaceholder = msg.source === 'placeholder';
      src.hidden = false;
      src.textContent = isPlaceholder ? 'placeholder' : 'alphabet binary';
      src.className = 'badge' + (isPlaceholder ? ' placeholder' : '');
      if (msg.warning) {
        warn.hidden = false;
        warn.textContent = '⚠ ' + msg.warning;
      } else {
        warn.hidden = true;
        warn.textContent = '';
      }
      btn.disabled = false; btn.textContent = 'Generate';
    }
  });
</script>
</body>
</html>`;
  }
}
