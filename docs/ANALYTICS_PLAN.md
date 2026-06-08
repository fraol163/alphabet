# Alphabet Language — Analytics/Telemetry Plan

## What We Track (Opt-in Only)

### Usage Metrics
- Number of programs run
- Most used builtins
- Most used stdlib modules
- Error frequencies
- Session duration

### Performance Metrics
- Compilation time
- Execution time
- Memory usage
- Startup time

### Feature Adoption
- Voice input usage
- REPL vs file execution
- Language distribution (en/am/es/fr/de)
- CLI command usage

## What We DON'T Track

- Source code content
- Variable names or values
- File paths
- Personal information
- Network requests content

## Implementation

### Opt-in Mechanism
```bash
# Enable telemetry
alphabet config set telemetry true

# Disable telemetry
alphabet config set telemetry false

# Check status
alphabet config get telemetry
```

### Data Format
```json
{
  "event": "program_run",
  "version": "2.3.5",
  "language": "en",
  "platform": "linux",
  "features_used": ["map", "filter", "lambda"],
  "execution_time_ms": 45
}
```

### Storage
- Local: ~/.alphabet/telemetry.json
- Remote: POST to telemetry.alphabet-lang.org (opt-in)
- Retention: 90 days

## Privacy

- **Opt-in only** — disabled by default
- **No PII** — no names, emails, or file paths
- **Aggregated** — only statistical summaries published
- **Transparent** — open source telemetry code
- **User control** — view and delete your data

## Benefits

- Prioritize features based on actual usage
- Identify and fix common errors
- Optimize performance bottlenecks
- Understand language distribution
- Guide development roadmap
