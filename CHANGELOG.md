# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- HTTPS support (using libcurl)

### Fixed

- Fixed getFPS function not reporting lag spikes properly

## [1.1.1] \(Hotfix) - 2019-03-09

### Fixed

- Fixed JSON module not parsing booleans properly

## [1.1.0] - 2019-03-09

### Added

- Full support for HTTP 1.1
- JSON support (GSC module, see unleashed\\json in bin)
- isArray function
- getEpochTime and epochTimeToString
- setSpectatedClient

### Changed

- Project renamed to CoD4: Unleashed
- Updated compile info
- Deprecated httpPostRequest

## 1.0 - 2016-08-13

- Initial release

[unreleased]: https://github.com/atrX/CoD4-Unleashed-Server/compare/1.1.0...HEAD
[1.1.1]: https://github.com/atrX/CoD4-Unleashed-Server/compare/1.1.0...1.1.1
[1.1.0]: https://github.com/atrX/CoD4-Unleashed-Server/compare/1.0...1.1.0
