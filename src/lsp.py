import sys
import json

def start_lsp():
    """A minimal LSP server for Alphabet"""
    while True:
        line = sys.stdin.readline()
        if not line: break
        
        # In a real LSP, we would parse JSON-RPC messages here
        # and provide autocompletion/diagnostics.
        # For now, we acknowledge the connection.
        if "initialize" in line:
            response = {
                "jsonrpc": "2.0",
                "id": 1,
                "result": {
                    "capabilities": {
                        "textDocumentSync": 1,
                        "completionProvider": {"resolveProvider": True}
                    }
                }
            }
            sys.stdout.write(f"Content-Length: {len(json.dumps(response))}

{json.dumps(response)}")
            sys.stdout.flush()

if __name__ == "__main__":
    start_lsp()
