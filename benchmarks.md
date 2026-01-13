# SignalForge Validation Performance Benchmarks

> Comparing SignalForge Validation (C extension) vs Laravel Validation vs Symfony Validator

## Environment

| Property | Value |
|----------|-------|
| PHP Version | 8.4.16 |
| SignalForge Extension | 1.0.0 |
| Laravel Validation | illuminate/validation v11.x |
| Symfony Validator | symfony/validator v7.x |
| Operating System | Linux |
| Benchmark Date | 2026-01-13 19:55:21 |

## Summary

Based on 1000-iteration benchmarks across all test scenarios:

- **SignalForge**: 1893316 ops/sec average
- **Laravel**: 5713 ops/sec average (SignalForge is **331.4x faster**)
- **Symfony**: 49436 ops/sec average (SignalForge is **38.3x faster**)

## Detailed Results

### Simple Validation (3 fields)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 2320185.6 |
| Laravel | 0.119 | 0.119 | 0.119 | 0.119 | 8435.8 |
| Symfony | 0.012 | 0.012 | 0.012 | 0.012 | 86722.7 |

*SignalForge is **275.0x faster** than Laravel, **26.8x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 3766478.3 |
| Laravel | 0.111 | 0.107 | 0.116 | 0.116 | 9049.7 |
| Symfony | 0.011 | 0.010 | 0.012 | 0.012 | 94046.0 |

*SignalForge is **416.2x faster** than Laravel, **40.0x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 3861302.0 |
| Laravel | 0.113 | 0.106 | 0.218 | 0.131 | 8824.2 |
| Symfony | 0.011 | 0.011 | 0.015 | 0.011 | 91489.8 |

*SignalForge is **437.6x faster** than Laravel, **42.2x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 3784667.6 |
| Laravel | 0.114 | 0.103 | 0.356 | 0.136 | 8795.4 |
| Symfony | 0.011 | 0.011 | 0.023 | 0.012 | 87314.6 |

*SignalForge is **430.3x faster** than Laravel, **43.3x faster** than Symfony*

### Complex Nested Validation (10 fields)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.002 | 0.002 | 0.002 | 0.002 | 612369.9 |
| Laravel | 0.285 | 0.285 | 0.285 | 0.285 | 3505.7 |
| Symfony | 0.042 | 0.042 | 0.042 | 0.042 | 23696.7 |

*SignalForge is **174.7x faster** than Laravel, **25.8x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.001 | 0.001 | 1027010.4 |
| Laravel | 0.265 | 0.253 | 0.293 | 0.293 | 3772.0 |
| Symfony | 0.039 | 0.039 | 0.039 | 0.039 | 25696.8 |

*SignalForge is **272.3x faster** than Laravel, **40.0x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.001 | 0.001 | 1052509.7 |
| Laravel | 0.284 | 0.251 | 0.528 | 0.410 | 3523.3 |
| Symfony | 0.040 | 0.038 | 0.056 | 0.045 | 25312.6 |

*SignalForge is **298.7x faster** than Laravel, **41.6x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.006 | 0.001 | 1031700.0 |
| Laravel | 0.263 | 0.247 | 0.475 | 0.286 | 3807.6 |
| Symfony | 0.039 | 0.037 | 0.059 | 0.042 | 25859.3 |

*SignalForge is **271.0x faster** than Laravel, **39.9x faster** than Symfony*

### Array/Wildcard Validation (3 items)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.003 | 0.003 | 0.003 | 0.003 | 309981.4 |
| Laravel | 0.255 | 0.255 | 0.255 | 0.255 | 3914.8 |
| Symfony | 0.039 | 0.039 | 0.039 | 0.039 | 25905.4 |

*SignalForge is **79.2x faster** than Laravel, **12.0x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.003 | 0.003 | 0.003 | 0.003 | 333288.9 |
| Laravel | 0.262 | 0.247 | 0.295 | 0.295 | 3816.0 |
| Symfony | 0.039 | 0.037 | 0.041 | 0.041 | 25942.0 |

*SignalForge is **87.3x faster** than Laravel, **12.8x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.003 | 0.003 | 0.003 | 0.003 | 338110.8 |
| Laravel | 0.252 | 0.240 | 0.318 | 0.279 | 3961.2 |
| Symfony | 0.039 | 0.037 | 0.059 | 0.043 | 25917.4 |

*SignalForge is **85.4x faster** than Laravel, **13.0x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.003 | 0.003 | 0.010 | 0.003 | 336153.4 |
| Laravel | 0.255 | 0.234 | 0.528 | 0.308 | 3914.8 |
| Symfony | 0.038 | 0.036 | 0.061 | 0.041 | 26619.0 |

*SignalForge is **85.9x faster** than Laravel, **12.6x faster** than Symfony*

### Email-Heavy Validation (5 emails)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 2375296.9 |
| Laravel | 0.135 | 0.135 | 0.135 | 0.135 | 7422.0 |
| Symfony | 0.015 | 0.015 | 0.015 | 0.015 | 67394.5 |

*SignalForge is **320.0x faster** than Laravel, **35.2x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 2452182.4 |
| Laravel | 0.171 | 0.125 | 0.356 | 0.356 | 5831.9 |
| Symfony | 0.015 | 0.014 | 0.015 | 0.015 | 68562.2 |

*SignalForge is **420.5x faster** than Laravel, **35.8x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 2469440.7 |
| Laravel | 0.130 | 0.121 | 0.220 | 0.161 | 7672.6 |
| Symfony | 0.016 | 0.014 | 0.073 | 0.022 | 63037.8 |

*SignalForge is **321.9x faster** than Laravel, **39.2x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.006 | 0.001 | 2383875.5 |
| Laravel | 0.126 | 0.121 | 0.212 | 0.143 | 7912.7 |
| Symfony | 0.015 | 0.014 | 0.028 | 0.020 | 67762.9 |

*SignalForge is **301.3x faster** than Laravel, **35.2x faster** than Symfony*

### Numeric-Heavy Validation (8 numbers)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.001 | 0.001 | 1721170.4 |
| Laravel | 0.248 | 0.248 | 0.248 | 0.248 | 4029.4 |
| Symfony | 0.025 | 0.025 | 0.025 | 0.025 | 39827.9 |

*SignalForge is **427.2x faster** than Laravel, **43.2x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.001 | 0.001 | 1854943.4 |
| Laravel | 0.243 | 0.239 | 0.258 | 0.258 | 4110.5 |
| Symfony | 0.026 | 0.025 | 0.031 | 0.031 | 39001.1 |

*SignalForge is **451.3x faster** than Laravel, **47.6x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.001 | 0.001 | 1897641.2 |
| Laravel | 0.243 | 0.237 | 0.305 | 0.256 | 4114.7 |
| Symfony | 0.025 | 0.024 | 0.029 | 0.026 | 40118.3 |

*SignalForge is **461.2x faster** than Laravel, **47.3x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.000 | 0.003 | 0.001 | 1930185.2 |
| Laravel | 0.242 | 0.235 | 0.347 | 0.252 | 4134.5 |
| Symfony | 0.025 | 0.024 | 0.044 | 0.029 | 39625.0 |

*SignalForge is **466.9x faster** than Laravel, **48.7x faster** than Symfony*

## Methodology

- Each benchmark includes 3 warmup runs before measurements
- Times are measured using `hrtime(true)` for nanosecond precision
- Statistics include: average, minimum, maximum, median, and 95th percentile
- Operations per second calculated as: iterations / (total_time_seconds)
- All validators configured with equivalent rules for fair comparison

## Test Scenarios

1. **Simple Validation**: 3 fields with basic rules (required, string, email, integer, min/max)
2. **Complex Nested Validation**: 10 fields in nested structure with various rules
3. **Array/Wildcard Validation**: Array of 3 items with wildcard rules
4. **Email-Heavy Validation**: 5 email fields to test email parsing performance
5. **Numeric-Heavy Validation**: 8 numeric fields with range constraints
