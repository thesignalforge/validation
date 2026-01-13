# SignalForge Validation Performance Benchmarks

> Comparing SignalForge Validation (C extension) vs YorCreative vs Laravel Validation vs Symfony Validator

## Environment

| Property | Value |
|----------|-------|
| PHP Version | 8.4.16 |
| SignalForge Extension | 1.0.0 |
| YorCreative | yorcreative/data-validation v1.x |
| Laravel Validation | illuminate/validation v11.x |
| Symfony Validator | symfony/validator v7.x |
| Operating System | Linux |
| Benchmark Date | 2026-01-13 20:22:13 |

## Summary

Based on 1000-iteration benchmarks across all test scenarios:

- **SignalForge**: 1588480 ops/sec average
- **YorCreative**: 42721 ops/sec average (SignalForge is **37.2x faster**)
- **Symfony**: 48314 ops/sec average (SignalForge is **32.9x faster**)
- **Laravel**: 4993 ops/sec average (SignalForge is **318.1x faster**)

## Detailed Results

### Simple Validation (3 fields)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 2325581.4 |
| YorCreative | 0.012 | 0.012 | 0.012 | 0.012 | 83801.2 |
| Laravel | 0.124 | 0.124 | 0.124 | 0.124 | 8050.7 |
| Symfony | 0.012 | 0.012 | 0.012 | 0.012 | 81149.1 |

*SignalForge is **27.8x faster** than YorCreative, **288.9x faster** than Laravel, **28.7x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 3500175.0 |
| YorCreative | 0.011 | 0.011 | 0.011 | 0.011 | 88354.1 |
| Laravel | 0.111 | 0.107 | 0.118 | 0.118 | 9038.1 |
| Symfony | 0.010 | 0.010 | 0.011 | 0.011 | 97816.7 |

*SignalForge is **39.6x faster** than YorCreative, **387.3x faster** than Laravel, **35.8x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 3627262.5 |
| YorCreative | 0.011 | 0.010 | 0.018 | 0.014 | 93213.2 |
| Laravel | 0.105 | 0.096 | 0.168 | 0.135 | 9501.9 |
| Symfony | 0.009 | 0.009 | 0.015 | 0.010 | 107031.9 |

*SignalForge is **38.9x faster** than YorCreative, **381.7x faster** than Laravel, **33.9x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.001 | 0.000 | 3864764.2 |
| YorCreative | 0.011 | 0.011 | 0.019 | 0.011 | 89128.3 |
| Laravel | 0.103 | 0.097 | 0.171 | 0.123 | 9705.7 |
| Symfony | 0.010 | 0.010 | 0.019 | 0.014 | 96851.2 |

*SignalForge is **43.4x faster** than YorCreative, **398.2x faster** than Laravel, **39.9x faster** than Symfony*

### Complex Nested Validation (10 fields)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.001 | 0.001 | 968992.2 |
| YorCreative | 0.052 | 0.052 | 0.052 | 0.052 | 19339.7 |
| Laravel | 0.261 | 0.261 | 0.261 | 0.261 | 3827.7 |
| Symfony | 0.034 | 0.034 | 0.034 | 0.034 | 29040.2 |

*SignalForge is **50.1x faster** than YorCreative, **253.2x faster** than Laravel, **33.4x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.001 | 0.001 | 1087192.9 |
| YorCreative | 0.053 | 0.052 | 0.059 | 0.059 | 18729.3 |
| Laravel | 0.256 | 0.248 | 0.271 | 0.271 | 3912.7 |
| Symfony | 0.036 | 0.034 | 0.041 | 0.041 | 27773.3 |

*SignalForge is **58.0x faster** than YorCreative, **277.9x faster** than Laravel, **39.1x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.002 | 0.001 | 1024569.2 |
| YorCreative | 0.057 | 0.048 | 0.085 | 0.080 | 17461.0 |
| Laravel | 0.280 | 0.244 | 0.443 | 0.403 | 3565.9 |
| Symfony | 0.037 | 0.032 | 0.256 | 0.051 | 26805.3 |

*SignalForge is **58.7x faster** than YorCreative, **287.3x faster** than Laravel, **38.2x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.005 | 0.001 | 1045006.3 |
| YorCreative | 0.056 | 0.054 | 0.081 | 0.063 | 17873.7 |
| Laravel | 0.270 | 0.250 | 0.455 | 0.300 | 3698.1 |
| Symfony | 0.036 | 0.035 | 0.073 | 0.042 | 27628.1 |

*SignalForge is **58.5x faster** than YorCreative, **282.6x faster** than Laravel, **37.8x faster** than Symfony*

### Array/Wildcard Validation (3 items)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.003 | 0.003 | 0.003 | 0.003 | 331565.0 |
| YorCreative | 0.046 | 0.046 | 0.046 | 0.046 | 21970.3 |
| Laravel | 0.250 | 0.250 | 0.250 | 0.250 | 4000.6 |
| Symfony | 0.035 | 0.035 | 0.035 | 0.035 | 28195.2 |

*SignalForge is **15.1x faster** than YorCreative, **82.9x faster** than Laravel, **11.8x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.003 | 0.003 | 0.003 | 0.003 | 338914.1 |
| YorCreative | 0.035 | 0.035 | 0.036 | 0.036 | 28211.4 |
| Laravel | 0.248 | 0.239 | 0.264 | 0.264 | 4040.3 |
| Symfony | 0.037 | 0.036 | 0.040 | 0.040 | 26905.7 |

*SignalForge is **12.0x faster** than YorCreative, **83.9x faster** than Laravel, **12.6x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.003 | 0.003 | 0.005 | 0.003 | 333431.1 |
| YorCreative | 0.035 | 0.033 | 0.044 | 0.038 | 28396.4 |
| Laravel | 0.256 | 0.238 | 0.361 | 0.347 | 3903.2 |
| Symfony | 0.037 | 0.035 | 0.046 | 0.042 | 27235.8 |

*SignalForge is **11.7x faster** than YorCreative, **85.4x faster** than Laravel, **12.2x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.003 | 0.003 | 0.010 | 0.005 | 319693.7 |
| YorCreative | 0.035 | 0.032 | 0.087 | 0.044 | 28876.8 |
| Laravel | 0.250 | 0.232 | 1.264 | 0.290 | 4006.3 |
| Symfony | 0.037 | 0.035 | 0.092 | 0.051 | 26690.5 |

*SignalForge is **11.1x faster** than YorCreative, **79.8x faster** than Laravel, **12.0x faster** than Symfony*

### Conditional Validation (4 fields with conditions)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.001 | 0.001 | 1919385.8 |
| YorCreative | 0.019 | 0.019 | 0.019 | 0.019 | 51369.0 |
| Laravel | 0.185 | 0.185 | 0.185 | 0.185 | 5399.3 |
| Symfony | 0.013 | 0.013 | 0.013 | 0.013 | 74537.9 |

*SignalForge is **37.4x faster** than YorCreative, **355.5x faster** than Laravel, **25.8x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.000 | 0.001 | 0.001 | 1992031.9 |
| YorCreative | 0.017 | 0.016 | 0.018 | 0.018 | 59868.1 |
| Laravel | 0.184 | 0.172 | 0.234 | 0.234 | 5432.6 |
| Symfony | 0.013 | 0.013 | 0.013 | 0.013 | 77392.2 |

*SignalForge is **33.3x faster** than YorCreative, **366.7x faster** than Laravel, **25.7x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.000 | 0.007 | 0.001 | 1721229.6 |
| YorCreative | 0.017 | 0.016 | 0.026 | 0.021 | 58746.8 |
| Laravel | 0.181 | 0.170 | 0.242 | 0.211 | 5521.4 |
| Symfony | 0.013 | 0.012 | 0.026 | 0.018 | 76031.2 |

*SignalForge is **29.3x faster** than YorCreative, **311.7x faster** than Laravel, **22.6x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.000 | 0.006 | 0.001 | 1914744.1 |
| YorCreative | 0.017 | 0.016 | 0.031 | 0.019 | 58431.4 |
| Laravel | 0.177 | 0.168 | 0.295 | 0.200 | 5638.8 |
| Symfony | 0.013 | 0.012 | 0.025 | 0.013 | 76930.9 |

*SignalForge is **32.8x faster** than YorCreative, **339.6x faster** than Laravel, **24.9x faster** than Symfony*

### Email-Heavy Validation (5 emails)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 2036659.9 |
| YorCreative | 0.015 | 0.015 | 0.015 | 0.015 | 68790.0 |
| Laravel | 0.146 | 0.146 | 0.146 | 0.146 | 6826.6 |
| Symfony | 0.015 | 0.015 | 0.015 | 0.015 | 67531.1 |

*SignalForge is **29.6x faster** than YorCreative, **298.3x faster** than Laravel, **30.2x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 2202643.2 |
| YorCreative | 0.014 | 0.014 | 0.014 | 0.014 | 71071.2 |
| Laravel | 0.130 | 0.124 | 0.139 | 0.139 | 7711.3 |
| Symfony | 0.018 | 0.014 | 0.034 | 0.034 | 56859.5 |

*SignalForge is **31.0x faster** than YorCreative, **285.6x faster** than Laravel, **38.7x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.000 | 0.000 | 2372591.8 |
| YorCreative | 0.014 | 0.014 | 0.024 | 0.014 | 69768.1 |
| Laravel | 0.127 | 0.123 | 0.151 | 0.145 | 7844.1 |
| Symfony | 0.015 | 0.014 | 0.024 | 0.021 | 66426.5 |

*SignalForge is **34.0x faster** than YorCreative, **302.5x faster** than Laravel, **35.7x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.000 | 0.000 | 0.006 | 0.000 | 2267733.1 |
| YorCreative | 0.014 | 0.014 | 0.036 | 0.018 | 68982.0 |
| Laravel | 0.129 | 0.114 | 0.382 | 0.153 | 7748.1 |
| Symfony | 0.015 | 0.014 | 0.026 | 0.015 | 68756.6 |

*SignalForge is **32.9x faster** than YorCreative, **292.7x faster** than Laravel, **33.0x faster** than Symfony*

### Numeric-Heavy Validation (8 numbers)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.001 | 0.001 | 1488095.2 |
| YorCreative | 0.030 | 0.030 | 0.030 | 0.030 | 33050.2 |
| Laravel | 0.245 | 0.245 | 0.245 | 0.245 | 4076.3 |
| Symfony | 0.026 | 0.026 | 0.026 | 0.026 | 38913.5 |

*SignalForge is **45.0x faster** than YorCreative, **365.1x faster** than Laravel, **38.2x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.001 | 0.001 | 1612383.1 |
| YorCreative | 0.029 | 0.029 | 0.030 | 0.030 | 34079.4 |
| Laravel | 0.248 | 0.238 | 0.262 | 0.262 | 4035.0 |
| Symfony | 0.026 | 0.025 | 0.033 | 0.033 | 38552.6 |

*SignalForge is **47.3x faster** than YorCreative, **399.6x faster** than Laravel, **41.8x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.001 | 0.001 | 1641416.2 |
| YorCreative | 0.031 | 0.030 | 0.042 | 0.035 | 32784.1 |
| Laravel | 0.251 | 0.239 | 0.408 | 0.276 | 3982.9 |
| Symfony | 0.026 | 0.025 | 0.041 | 0.029 | 38572.7 |

*SignalForge is **50.1x faster** than YorCreative, **412.1x faster** than Laravel, **42.6x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.001 | 0.001 | 0.006 | 0.001 | 1694056.7 |
| YorCreative | 0.028 | 0.027 | 0.049 | 0.032 | 35571.1 |
| Laravel | 0.242 | 0.214 | 0.420 | 0.265 | 4130.5 |
| Symfony | 0.024 | 0.023 | 0.041 | 0.029 | 41017.1 |

*SignalForge is **47.6x faster** than YorCreative, **410.1x faster** than Laravel, **41.3x faster** than Symfony*

### Large Scale Validation (1000 fields)

#### 1 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.074 | 0.074 | 0.074 | 0.074 | 13482.7 |
| YorCreative | 4.939 | 4.939 | 4.939 | 4.939 | 202.5 |
| Laravel | 39.078 | 39.078 | 39.078 | 39.078 | 25.6 |
| Symfony | 2.772 | 2.772 | 2.772 | 2.772 | 360.7 |

*SignalForge is **66.6x faster** than YorCreative, **526.9x faster** than Laravel, **37.4x faster** than Symfony*

#### 10 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.076 | 0.073 | 0.087 | 0.087 | 13157.9 |
| YorCreative | 4.936 | 4.863 | 5.000 | 5.000 | 202.6 |
| Laravel | 39.714 | 39.485 | 39.945 | 39.945 | 25.2 |
| Symfony | 2.796 | 2.772 | 2.832 | 2.832 | 357.6 |

*SignalForge is **65.0x faster** than YorCreative, **522.5x faster** than Laravel, **36.8x faster** than Symfony*

#### 100 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.075 | 0.072 | 0.097 | 0.084 | 13413.2 |
| YorCreative | 4.936 | 4.738 | 5.494 | 5.238 | 202.6 |
| Laravel | 40.774 | 39.258 | 44.404 | 43.226 | 24.5 |
| Symfony | 2.928 | 2.620 | 4.513 | 3.119 | 341.5 |

*SignalForge is **66.2x faster** than YorCreative, **546.9x faster** than Laravel, **39.3x faster** than Symfony*

#### 1000 Iterations

| Library | Avg (ms) | Min (ms) | Max (ms) | P95 (ms) | Ops/sec |
|---------|----------|----------|----------|----------|--------|
| SignalForge | 0.075 | 0.071 | 0.122 | 0.082 | 13360.8 |
| YorCreative | 5.529 | 4.927 | 8.561 | 6.243 | 180.9 |
| Laravel | 42.739 | 39.262 | 48.323 | 44.870 | 23.4 |
| Symfony | 3.074 | 2.789 | 4.414 | 3.291 | 325.3 |

*SignalForge is **73.9x faster** than YorCreative, **571.0x faster** than Laravel, **41.1x faster** than Symfony*

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
4. **Conditional Validation**: 4 fields with conditional rules (required_if equivalent)
5. **Email-Heavy Validation**: 5 email fields to test email parsing performance
6. **Numeric-Heavy Validation**: 8 numeric fields with range constraints
7. **Large Scale Validation**: 1000 fields to test scalability
