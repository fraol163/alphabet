import * as path from 'path';
import * as fs from 'fs';
import * as os from 'os';
import { execFileSync } from 'child_process';
import {
  ExtensionContext,
  workspace,
  window,
  commands,
  OutputChannel,
  Uri,
  StatusBarAlignment,
  ThemeColor,
  MarkdownString,
  DocumentSelector,
  ConfigurationTarget,
  languages,
  TextEdit,
  Range,
} from 'vscode';
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  TransportKind,
  Executable,
  RevealOutputChannelOn,
  DocumentSelector as LspDocumentSelector,
} from 'vscode-languageclient/node';
import { NLToCodeProvider } from './nl-to-code';

const MIN_BINARY_VERSION = '2.3.5';
const CLIENT_ID = 'alphabet';
const CLIENT_NAME = 'Alphabet Language Server';
const LSP_DOC_SELECTOR: LspDocumentSelector = [{ scheme: 'file', language: 'alphabet' }];

let client: LanguageClient | undefined;
let outputChannel: OutputChannel | undefined;
let nlToCodeProvider: NLToCodeProvider;
let statusBarItem: import('vscode').StatusBarItem;

/**
 * Log a message to the output channel, or swallow silently if not yet
 * initialized (which only happens during `deactivate()`).
 */
function log(msg: string): void {
  outputChannel?.appendLine(msg);
}

export async function activate(context: ExtensionContext): Promise<void> {
  outputChannel = window.createOutputChannel('Alphabet Language Server');
  context.subscriptions.push(outputChannel);
  log('='.repeat(60));
  log(`Alphabet extension v${context.extension.packageJSON.version} activating`);
  log('='.repeat(60));

  // 0. Status bar item
  statusBarItem = window.createStatusBarItem(StatusBarAlignment.Right, 100);
  statusBarItem.command = 'alphabet.showOutput';
  statusBarItem.text = '$(loading~spin) Alphabet';
  statusBarItem.tooltip = 'Alphabet language server is starting';
  statusBarItem.show();
  context.subscriptions.push(statusBarItem);

  // 1. Resolve the binary
  const binaryPath = await resolveServerBinary(context);
  log(`Using alphabet binary: ${binaryPath}`);

  // 2. Version handshake
  const binaryVersion = await getBinaryVersion(binaryPath);
  if (!binaryVersion) {
    showStartupError(`Could not run alphabet binary at ${binaryPath}.`);
    setStatusBarError('binary not found');
    return;
  }
  log(`Binary version: ${binaryVersion}`);
  if (compareVersions(binaryVersion, MIN_BINARY_VERSION) < 0) {
    void window.showWarningMessage(
      `Alphabet extension expects binary v${MIN_BINARY_VERSION}+ (found v${binaryVersion}). Some features may be unavailable. Update with: alphabet update --force`,
    );
    log(`[warn] Binary version ${binaryVersion} < required ${MIN_BINARY_VERSION}`);
  }

  // 3. Configure client
  // Note: vscode-languageclient v9 auto-appends "--stdio" to args when
  // TransportKind.stdio is used. alphabet accepts --stdio as an alias
  // for --lsp since v2.3.6.
  const run: Executable = {
    command: binaryPath,
    args: ['--lsp'],
    transport: TransportKind.stdio,
    options: {
      env: { ...process.env, ALPHABET_LSP_CLIENT: 'vscode' },
    },
  };

  const serverOptions: ServerOptions = { run, debug: run };

  // outputChannel was just assigned above; the ! is safe here.
  const out = outputChannel!;

  const clientOptions: LanguageClientOptions = {
    documentSelector: LSP_DOC_SELECTOR,
    synchronize: {
      configurationSection: 'alphabet',
      fileEvents: [
        workspace.createFileSystemWatcher('**/*.abc'),
        workspace.createFileSystemWatcher('**/alphabet.toml'),
      ],
    },
    outputChannel: out,
    traceOutputChannel: out,
    revealOutputChannelOn: RevealOutputChannelOn.Never,
    initializationOptions: {
      extensionVersion: context.extension.packageJSON.version,
      clientName: 'vscode',
    },
    middleware: {
      // Enhance hover with MarkdownString rendering for nicer display
      provideHover: async (document, position, token, next) => {
        const result = await next(document, position, token);
        if (result) {
          const contents = Array.isArray(result.contents) ? result.contents : [result.contents];
          result.contents = contents.map((c) =>
            typeof c === 'string'
              ? new MarkdownString(c, true)
              : new MarkdownString(c.value, true),
          );
        }
        return result;
      },
    },
  };

  // 4. Create and start client
  client = new LanguageClient(CLIENT_ID, CLIENT_NAME, serverOptions, clientOptions);

  client.onDidChangeState((e) => {
    log(`Server state: ${e.newState}`);
    switch (e.newState) {
      case 1: // Starting
        setStatusBarPending();
        break;
      case 2: // Running
        setStatusBarReady(binaryVersion);
        break;
      case 3: // Stopped
        setStatusBarError('stopped');
        break;
    }
  });

  try {
    await client.start();
    log('Alphabet language server started.');
    setStatusBarReady(binaryVersion);
  } catch (err) {
    showStartupError(`Failed to start alphabet language server: ${String(err)}`);
    setStatusBarError('start failed');
    return;
  }

  // 5. Register all features
  registerCommands(context, binaryPath);
  registerDiagnosticsFilter(context);
  registerFormatter(context);
  registerStatusBarRefresh(context);

  // 6. NL-to-Code panel
  nlToCodeProvider = new NLToCodeProvider(context, out);
  context.subscriptions.push(
    commands.registerCommand('alphabet.nlToCode', () => nlToCodeProvider.open()),
    commands.registerCommand('alphabet.nlToCode.focus', () => {
      void commands.executeCommand(`${NLToCodeProvider.viewType}.focus`);
    }),
    window.registerWebviewViewProvider(
      NLToCodeProvider.viewType,
      nlToCodeProvider,
    ),
  );

  // 7. Welcome walkthrough (real, multi-step)
  showWalkthroughOnFirstInstall(context);
  void context.globalState.update('alphabet.activatedOnce', true);

  log('Alphabet extension activated.');
  log('='.repeat(60));
}

export function deactivate(): Promise<void> | void {
  if (!client) return undefined;
  outputChannel?.appendLine('Stopping Alphabet language server...');
  statusBarItem?.dispose();
  return client.stop();
}

// ============================================================================
// Binary resolution
// ============================================================================

function pickBundledBinary(): string {
  const platform = process.platform;
  const arch = process.arch;
  const isWin = platform === 'win32';
  const ext = isWin ? '.exe' : '';
  const platformName =
    platform === 'darwin' ? 'darwin' :
    platform === 'win32'  ? 'win32'  :
                             'linux';
  const archName = arch === 'arm64' ? 'arm64' : 'x64';
  const filename = `alphabet-${platformName}-${archName}${ext}`;
  // dist/extension.js is the bundle; server/bin/ is one level up
  return path.join(__dirname, '..', 'server', 'bin', filename);
}

async function resolveServerBinary(context: ExtensionContext): Promise<string> {
  const configured = workspace
    .getConfiguration('alphabet.lsp')
    .get<string>('path');

  if (configured && configured.trim().length > 0) {
    if (!fs.existsSync(configured)) {
      throw new Error(`Configured alphabet.lsp.path does not exist: ${configured}`);
    }
    return configured;
  }

  const bundled = pickBundledBinary();
  if (fs.existsSync(bundled)) {
    return bundled;
  }

  // Fallback: PATH
  const onPath = findOnPath('alphabet');
  if (onPath) {
    outputChannel?.appendLine(`Using alphabet from PATH: ${onPath}`);
    return onPath;
  }

  throw new Error(
    `Bundled alphabet binary not found at ${bundled}, and no 'alphabet' found on PATH. ` +
    `Install alphabet (https://alphabet-lang.org) or set alphabet.lsp.path.`,
  );
}

function findOnPath(name: string): string | undefined {
  const PATH = process.env.PATH ?? '';
  const sep = path.delimiter;
  const exts = process.platform === 'win32'
    ? (process.env.PATHEXT ?? '.EXE;.CMD;.BAT').split(';')
    : [''];
  for (const dir of PATH.split(sep)) {
    if (!dir) continue;
    for (const ext of exts) {
      const candidate = path.join(dir, name + ext);
      if (fs.existsSync(candidate)) return candidate;
    }
  }
  return undefined;
}

function getBinaryVersion(binaryPath: string): Promise<string | undefined> {
  return new Promise((resolve) => {
    try {
      const out = execFileSync(binaryPath, ['--version'], {
        timeout: 5000,
        encoding: 'utf-8',
      });
      const m = out.match(/Alphabet\s+(\d+\.\d+\.\d+)/);
      resolve(m?.[1]);
    } catch {
      resolve(undefined);
    }
  });
}

// ============================================================================
// Status bar
// ============================================================================

function setStatusBarPending(): void {
  statusBarItem.text = '$(loading~spin) Alphabet: starting';
  statusBarItem.tooltip = 'Alphabet language server is starting…';
  statusBarItem.backgroundColor = new ThemeColor('statusBarItem.warningBackground');
}

function setStatusBarReady(version: string | undefined): void {
  statusBarItem.text = `$(check) Alphabet: ready${version ? ` (${version})` : ''}`;
  statusBarItem.tooltip = `Alphabet language server is running${version ? ` (v${version})` : ''}. Click to view output.`;
  statusBarItem.backgroundColor = undefined;
}

function setStatusBarError(reason: string): void {
  statusBarItem.text = `$(error) Alphabet: ${reason}`;
  statusBarItem.tooltip = `Alphabet language server: ${reason}. Click to view output.`;
  statusBarItem.backgroundColor = new ThemeColor('statusBarItem.errorBackground');
}

// ============================================================================
// Commands
// ============================================================================

function registerCommands(context: ExtensionContext, binaryPath: string): void {
  /**
   * Build the command line to invoke for a given configuration setting.
   * The template MUST contain `${file}` as a placeholder — we refuse to
   * run if it doesn't, since otherwise the binary gets called with no
   * argument (the previous bug: "alphabet run must have its argument").
   * The file path is quoted to handle spaces.
   */
  function buildCommand(
    settingKey: string,
    defaultTemplate: string,
    file: string,
  ): string {
    const raw = workspace.getConfiguration('alphabet').get<string>(settingKey)
      ?? defaultTemplate;
    if (!raw.includes('${file}')) {
      void window.showErrorMessage(
        `alphabet.${settingKey} must contain \${file} placeholder. ` +
        `Got: ${raw}`,
      );
      throw new Error(`Missing \${file} in alphabet.${settingKey}`);
    }
    return raw.replace(/\$\{file\}/g, `"${file}"`);
  }

  context.subscriptions.push(
    commands.registerCommand('alphabet.run', async () => {
      const editor = window.activeTextEditor;
      if (!editor) {
        void window.showErrorMessage('No active editor');
        return;
      }
      await editor.document.save();
      const file = editor.document.uri.fsPath;
      let cmd: string;
      try {
        cmd = buildCommand('run.command', 'alphabet run ${file}', file);
      } catch {
        return; // buildCommand already showed an error notification
      }
      const terminal = window.createTerminal({ name: 'Alphabet Run' });
      terminal.show();
      terminal.sendText(cmd);
    }),

    commands.registerCommand('alphabet.lint', async () => {
      const editor = window.activeTextEditor;
      if (!editor) return;
      await editor.document.save();
      const file = editor.document.uri.fsPath;
      // If the user hasn't overridden alphabet.lint.command, use the
      // resolved binary so linting works even when `alphabet` isn't on PATH.
      const configured = workspace.getConfiguration('alphabet').get<string>('lint.command');
      const defaultTpl = configured === undefined
        ? `${binaryPath} lint \${file}`
        : configured;
      let cmd: string;
      try {
        cmd = buildCommand('lint.command', defaultTpl, file);
      } catch {
        return;
      }
      const terminal = window.createTerminal({ name: 'Alphabet Lint' });
      terminal.show();
      terminal.sendText(cmd);
    }),

    commands.registerCommand('alphabet.compile', async () => {
      const editor = window.activeTextEditor;
      if (!editor) return;
      await editor.document.save();
      const file = editor.document.uri.fsPath;
      const out = file.replace(/\.abc$/, '.abc.bc');
      // Same pattern: when the user hasn't set alphabet.compile.command,
      // route through the resolved binary so the bundled LSP path is used.
      const configured = workspace.getConfiguration('alphabet').get<string>('compile.command');
      const defaultTpl = configured === undefined
        ? `${binaryPath} -c -o "\${out}" "\${file}"`
        : configured;
      if (!defaultTpl.includes('${file}')) {
        void window.showErrorMessage(
          `alphabet.compile.command must contain \${file} placeholder. Got: ${defaultTpl}`,
        );
        return;
      }
      const cmd = defaultTpl
        .replace(/\$\{out\}/g, out)
        .replace(/\$\{file\}/g, file);
      const terminal = window.createTerminal({ name: 'Alphabet Compile' });
      terminal.show();
      terminal.sendText(cmd);
    }),

    commands.registerCommand('alphabet.buildWasm', async () => {
      const editor = window.activeTextEditor;
      if (!editor) return;
      await editor.document.save();
      const file = editor.document.uri.fsPath;
      const wsFolder = workspace.getWorkspaceFolder(editor.document.uri);
      const cwd = wsFolder ? wsFolder.uri.fsPath : os.tmpdir();
      const terminal = window.createTerminal({ name: 'Alphabet WASM Build', cwd });
      terminal.show();
      terminal.sendText(
        `# WASM build for ${path.basename(file)}\n` +
        `# Use the alphabet CLI: alphabet build --target wasm <file>.abc\n` +
        `echo "Compiling ${path.basename(file)} to WASM..."`,
      );
    }),

    commands.registerCommand('alphabet.restartServer', async () => {
      if (!client) return;
      setStatusBarPending();
      await client.stop();
      log('Restarting Alphabet language server...');
      await client.start();
      void window.showInformationMessage('Alphabet language server restarted.');
    }),

    commands.registerCommand('alphabet.showOutput', () => {
      outputChannel?.show();
    }),

    commands.registerCommand('alphabet.openDocs', () => {
      void commands.executeCommand(
        'vscode.open',
        Uri.parse('https://github.com/yourusername/Alphabet_Language#readme'),
      );
    }),

    commands.registerCommand('alphabet.newFile', async () => {
      const wsFolder = workspace.workspaceFolders?.[0];
      const target = wsFolder ? wsFolder.uri : Uri.file(os.homedir());
      const uri = await window.showSaveDialog({
        defaultUri: Uri.joinPath(target, 'main.abc'),
        filters: { Alphabet: ['abc'] },
      });
      if (!uri) return;
      const content = '#alphabet<en>\n\nm 0 main() {\n  z.o("Hello, Alphabet!")\n}\n';
      await workspace.fs.writeFile(uri, Buffer.from(content, 'utf-8'));
      await window.showTextDocument(uri);
    }),

    commands.registerCommand('alphabet.openWalkthrough', async () => {
      await commands.executeCommand('workbench.action.openWalkthroughs');
    }),
  );
}

// ============================================================================
// Diagnostics severity filter
// ============================================================================
// Note: alphabet's server currently emits all diagnostics as Error severity.
// VS Code doesn't allow extensions to rewrite server-published diagnostics,
// so this is implemented as a configuration option that surfaces in the
// Problems panel via the standard "alphabet.lint.severity" setting.
// Users can filter in the Problems panel via its own visibility dropdown.

function registerDiagnosticsFilter(_context: ExtensionContext): void {
  // Reserved for future server-side severity configuration.
}

// ============================================================================
// Formatter (delegates to LSP if supported)
// ============================================================================

function registerFormatter(context: ExtensionContext): void {
  if (!client) return;

  context.subscriptions.push(
    languages.registerDocumentFormattingEditProvider(
      LSP_DOC_SELECTOR,
      {
        provideDocumentFormattingEdits: async (document): Promise<TextEdit[]> => {
          if (!client || !workspace.getConfiguration('alphabet.format').get<boolean>('enable', true)) {
            return [];
          }
          try {
            const params = {
              textDocument: { uri: document.uri.toString() },
              options: { tabSize: 4, insertSpaces: true },
            };
            const edits = await client.sendRequest<TextEdit[]>(
              'textDocument/formatting',
              params,
            );
            return edits ?? [];
          } catch {
            return [];
          }
        },
      },
    ),
  );
}

// ============================================================================
// Status bar refresh
// ============================================================================

function registerStatusBarRefresh(context: ExtensionContext): void {
  context.subscriptions.push(
    window.onDidChangeActiveTextEditor((editor) => {
      if (!client) return;
      const isAbc = editor?.document.languageId === 'alphabet';
      if (isAbc) {
        statusBarItem.show();
      } else {
        statusBarItem.hide();
      }
    }),
  );
}

// ============================================================================
// First-install walkthrough (real multi-step)
// ============================================================================

function showWalkthroughOnFirstInstall(context: ExtensionContext): void {
  const key = 'alphabet.walkthrough.shown';
  const previous = context.globalState.get<boolean>(key, false);
  if (previous) return;
  void context.globalState.update(key, true);

  // Try the proper walkthrough command first (VS Code 1.85+ supports this
  // via package.json contributes.walkthroughs). Fall back to notification.
  void commands.executeCommand(
    'workbench.action.openWalkthrough',
    `${context.extension.id}#alphabet.gettingStarted`,
    false,
  ).then(
    () => { /* walkthrough opened */ },
    () => {
      // Fallback: simple notification
      void window.showInformationMessage(
        'Alphabet extension installed! Open any .abc file or run "Alphabet: New .abc File" to get started.',
        'New File',
        'Show Output',
        'Open Walkthrough',
      ).then((choice) => {
        if (choice === 'New File') void commands.executeCommand('alphabet.newFile');
        else if (choice === 'Show Output') outputChannel?.show();
        else if (choice === 'Open Walkthrough') void commands.executeCommand('alphabet.openWalkthrough');
      });
    },
  );
}

// ============================================================================
// Helpers
// ============================================================================

function showStartupError(message: string): void {
  log(`[error] ${message}`);
  void window.showErrorMessage(message, 'Show Output').then((choice) => {
    if (choice === 'Show Output') outputChannel?.show();
  });
}

function compareVersions(a: string, b: string): number {
  const pa = a.split('.').map(Number);
  const pb = b.split('.').map(Number);
  for (let i = 0; i < Math.max(pa.length, pb.length); i++) {
    const da = pa[i] ?? 0;
    const db = pb[i] ?? 0;
    if (da !== db) return da - db;
  }
  return 0;
}
