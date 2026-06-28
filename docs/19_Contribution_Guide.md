# 19 — Contribution Guide

## CodeStudio Recorder — Contributor Workflow

---

## Overview

Welcome to the CodeStudio Recorder project. This guide covers everything you need to contribute — from branching strategy to pull requests, commit standards, and project governance.

---

## Getting Started

### 1. Fork & Clone

```bash
# Fork on GitHub, then:
git clone https://github.com/YOUR_USERNAME/CodeStudioRecorder.git
cd CodeStudioRecorder
git remote add upstream https://github.com/codestudio/CodeStudioRecorder.git
```

### 2. Set Up Dev Environment

Follow the Prerequisites section in `10_Build_Deployment.md`.

```powershell
# Build native engine (Debug)
powershell scripts/build.ps1 -Config Debug -NativeOnly

# Run Flutter app
flutter pub get
flutter run -d windows
```

### 3. Run Tests

```bash
# C++ tests
ctest --test-dir build --config Debug

# Flutter tests
flutter test
```

---

## Branching Strategy

```
main          ← stable, always releasable
develop       ← integration branch for next release
feature/*     ← feature branches (from develop)
fix/*         ← bug fix branches (from develop or main)
hotfix/*      ← critical fixes (from main)
release/vX.Y  ← release preparation (from develop)
```

### Branch Naming

```
feature/nvenc-encoder-support
feature/flutter-history-screen
fix/audio-drift-long-recordings
hotfix/crash-on-window-close
```

---

## Commit Standards

### Conventional Commits

All commits must follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]
[optional footer]
```

### Types

| Type | When to use |
|---|---|
| `feat` | New feature |
| `fix` | Bug fix |
| `perf` | Performance improvement |
| `refactor` | Code change without feature/fix |
| `test` | Adding or fixing tests |
| `docs` | Documentation only |
| `build` | Build system changes |
| `ci` | CI/CD pipeline changes |
| `chore` | Dependency updates, minor maintenance |

### Examples

```
feat(encoder): add AMD AMF encoder support
fix(audio): prevent drift on recordings over 30 minutes
perf(capture): use lock-free queue for frame pipeline
docs(api): document FFI export error codes
test(state-machine): add invalid transition tests
```

---

## Pull Request Process

### PR Checklist

Before opening a PR:

- [ ] Branch is up-to-date with `develop`
- [ ] All tests pass (`ctest` + `flutter test`)
- [ ] `flutter analyze` reports 0 issues
- [ ] Follows coding standards (`16_Coding_Standards.md`)
- [ ] New features have tests
- [ ] Documentation updated if API changed
- [ ] Commit messages follow conventional commits

### PR Title Format

```
feat(encoder): add Quick Sync H.265 support
fix(audio): resolve WASAPI loopback silence gaps
```

### PR Description Template

```markdown
## Summary
Brief description of what this PR does.

## Motivation
Why is this change needed?

## Changes
- Added `QuickSyncEncoder` class with H.265 support
- Updated `EncoderFactory::probeQuickSync()` to detect QSV
- Added unit tests for QuickSyncEncoder initialization

## Testing
How was this tested? On what hardware?

## Breaking Changes
None / List any breaking changes
```

---

## Issue Reporting

### Bug Report Template

```markdown
**Bug Description**
A clear description of the bug.

**Steps to Reproduce**
1. Open CodeStudio
2. Select window capture
3. Press record
4. ...

**Expected Behavior**
What should happen.

**Actual Behavior**
What actually happens.

**Environment**
- Windows version:
- GPU: (NVIDIA RTX xxx / Intel HD xxx / AMD RX xxx)
- CodeStudio version:
- Recording config: (resolution, fps, codec)
```

### Feature Request Template

```markdown
**Feature Description**
What feature would you like?

**Use Case**
Why do you need this? What problem does it solve?

**Proposed Implementation** (optional)
Any ideas on how it could be implemented?
```

---

## Project Governance

### Maintainers

The project is maintained by the core team. Maintainers:
- Review and merge PRs to `main` and `develop`
- Cut releases and manage changelogs
- Triage issues and set roadmap priorities

### Becoming a Contributor

All contributions are welcome! Regular contributors with quality PRs may be invited to become maintainers.

### Code of Conduct

This project follows the [Contributor Covenant Code of Conduct](https://www.contributor-covenant.org/). Be respectful, constructive, and collaborative.

---

## Release Notes

Releases are generated from conventional commits automatically. The CHANGELOG is maintained via `git-cliff` or similar tooling:

```bash
# Generate changelog for next release
git-cliff --tag v1.1.0 --output CHANGELOG.md
```

---

*Last updated: 2025 | Module 19 of 19*
