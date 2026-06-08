# Alphabet Language — Release Process

## Version Numbering

Alphabet follows Semantic Versioning (Semver):
```
MAJOR.MINOR.PATCH
  │     │     └── Bug fixes, no breaking changes
  │     └──────── New features, backward compatible
  └────────────── Breaking changes
```

## Release Checklist

### Pre-release
- [ ] All tests pass (91+ tests)
- [ ] All examples run correctly
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] VERSION file updated
- [ ] No old version references remain
- [ ] Binary builds on Linux + macOS

### Release
- [ ] Create git tag: `git tag v2.4.0`
- [ ] Push tag: `git push origin v2.4.0`
- [ ] GitHub Actions builds binaries
- [ ] GitHub Actions creates release
- [ ] Docker image published
- [ ] Homebrew formula updated
- [ ] AUR package updated

### Post-release
- [ ] Announce on Twitter
- [ ] Announce on Discord
- [ ] Update documentation site
- [ ] Blog post about new features

## Release Cadence

| Type | Frequency | Example |
|------|-----------|---------|
| Patch | As needed | v2.3.6 (bug fix) |
| Minor | Monthly | v2.4.0 (new features) |
| Major | Yearly | v3.0.0 (breaking changes) |

## Hotfix Process

1. Create hotfix branch from tag
2. Fix the issue
3. Test thoroughly
4. Create patch release (v2.3.6)
5. Announce hotfix

## Nightly Builds

- Built automatically every night
- Available on GitHub Releases
- Tagged as `nightly-YYYYMMDD`
- Pre-release (not stable)

## Binary Distribution

| Platform | Format | Status |
|----------|--------|--------|
| Linux x86_64 | .tar.gz | ✅ CI |
| macOS x86_64 | .tar.gz | ✅ CI |
| macOS ARM64 | .tar.gz | 🔄 Planned |
| Windows x86_64 | .zip | 🔄 Planned |
| Docker | image | ✅ Ready |
| Homebrew | formula | ✅ Ready |
| AUR | PKGBUILD | ✅ Ready |
| Snap | snap | ✅ Ready |
