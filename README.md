# Signalforge Validation - PHP C Extension

High-performance PHP C extension for input validation. Provides the same API as the pure PHP library but with significantly better performance for CPU-bound validation tasks.

## Requirements

- PHP 8.2+ (NTS and ZTS)
- PCRE2 library (libpcre2-8)
- Linux or macOS

## Installation

### Quick Build (Recommended)

Use the included build script which handles multiple PHP versions:

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install php8.4-dev libpcre2-dev   # For PHP 8.4
sudo apt-get install php8.5-dev libpcre2-dev   # For PHP 8.5

# Build for a specific PHP version
./build.sh 8.4          # Build for PHP 8.4
./build.sh 8.5          # Build for PHP 8.5
./build.sh              # Build for default PHP version

# Run tests
./build.sh test 8.4     # Test with PHP 8.4
./build.sh test 8.5     # Test with PHP 8.5

# Clean build artifacts
./build.sh clean
```

### Manual Build

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install php-dev libpcre2-dev

# Install dependencies (macOS)
brew install pcre2

# Build the extension
phpize
./configure --enable-signalforge-validation
make
make test

# Install
sudo make install

# Enable the extension
echo "extension=signalforge_validation.so" | sudo tee /etc/php/8.4/cli/conf.d/signalforge_validation.ini
```

### Building for Multiple PHP Versions

PHP extensions are version-specific. To support multiple PHP versions, build separately for each:

```bash
# Build for PHP 8.4
./build.sh clean
./build.sh 8.4
sudo cp modules/signalforge_validation.so /usr/lib/php/20240924/

# Build for PHP 8.5
./build.sh clean
./build.sh 8.5
sudo cp modules/signalforge_validation.so /usr/lib/php/20250925/
```

## Usage

```php
<?php
use Signalforge\Validation\Validator;

$validator = new Validator([
    'email' => ['required', 'email'],
    'name' => ['required', 'string', ['min', 2], ['max', 100]],
    'age' => ['nullable', 'integer', ['between', 18, 120]],
]);

$result = $validator->validate([
    'email' => 'user@example.com',
    'name' => 'John',
    'age' => 25,
]);

if ($result->valid()) {
    $data = $result->validated();
    // Process validated data
} else {
    $errors = $result->errors();
    // Handle validation errors
}
```

### Static Factory

```php
$validator = Validator::make($data, $rules);
$result = $validator->validate($data);
```

## Validation Result

```php
$result->valid();           // bool - true if validation passed
$result->failed();          // bool - true if validation failed
$result->errors();          // array - all errors grouped by field
$result->validated();       // array - validated data
$result->errorsFor('email'); // array - errors for specific field
$result->hasError('email'); // bool - check if field has errors
```

## Available Rules

### Presence Rules
- `required` - Field must be present and not empty
- `nullable` - Allow null/empty values
- `filled` - If present, must not be empty
- `present` - Field must exist (can be empty)

### Type Rules
- `string` - Must be a string
- `integer` - Must be an integer
- `numeric` - Must be numeric
- `boolean` - Must be boolean
- `array` - Must be an array

### String Rules
- `['min', n]` - Minimum length
- `['max', n]` - Maximum length
- `['between', min, max]` - Length between min and max
- `['regex', pattern]` - Must match regex
- `['not_regex', pattern]` - Must not match regex
- `alpha` - Only alphabetic characters
- `alpha_num` - Only alphanumeric characters
- `alpha_dash` - Alphanumeric plus dashes and underscores
- `lowercase` - Must be all lowercase
- `uppercase` - Must be all uppercase
- `['starts_with', str]` - Must start with string
- `['ends_with', str]` - Must end with string
- `['contains', str]` - Must contain string

### Numeric Rules
- `['gt', n]` - Greater than
- `['gte', n]` - Greater than or equal
- `['lt', n]` - Less than
- `['lte', n]` - Less than or equal

### Array Rules
- `distinct` - All values must be unique

### Format Rules
- `email` - Valid email address
- `url` - Valid URL
- `ip` - Valid IP address (v4 or v6)
- `uuid` - Valid UUID
- `json` - Valid JSON string
- `date` - Valid date string
- `['date_format', format]` - Date matches format

### Comparison Rules
- `['in', [...]]` - Value must be in list
- `['not_in', [...]]` - Value must not be in list
- `['same', field]` - Must match another field
- `['different', field]` - Must differ from another field
- `confirmed` - Must have matching `{field}_confirmation`

### Date Rules
- `['after', field]` - Date after another field
- `['before', field]` - Date before another field
- `['after_or_equal', field]` - Date after or equal
- `['before_or_equal', field]` - Date before or equal

### Regional Rules
- `oib` - Croatian personal ID (OIB)
- `phone` - Valid phone number
- `iban` - Valid IBAN
- `vat_eu` - EU VAT number

## Conditional Validation

```php
$validator = new Validator([
    'type' => ['required', ['in', ['personal', 'business']]],
    'company' => [
        ['when', ['type', '=', 'business'], [
            'required', 'string', ['min', 2],
        ]],
    ],
]);
```

### Condition Syntax

**Cross-field:**
```php
['field', '=', 'value']
['field', '!=', 'value']
['field', '>', 100]
['field', 'in', ['a', 'b', 'c']]
['field', 'filled']
['field', 'empty']
```

**Self-referential:**
```php
['@length', '>=', 256]
['@value', '=', 'special']
['@empty']
['@filled']
```

**Compound (multiple conditions):**
```php
// All conditions must be true (AND)
['and', ['type', '=', 'business'], ['country', '=', 'HR']]

// At least one condition must be true (OR)
['or', ['role', '=', 'admin'], ['role', '=', 'moderator']]

// Nested conditions
['and',
    ['type', '=', 'business'],
    ['or', ['country', '=', 'HR'], ['country', '=', 'SI']]
]

// Multiple conditions with self-referential checks
['and', ['@filled'], ['@length', '>=', 5], ['@length', '<=', 100]]
```

**Full example with compound conditions:**
```php
$validator = new Validator([
    'type' => ['required', ['in', ['personal', 'business']]],
    'country' => ['required', 'string'],
    'vat_number' => [
        // Required only for EU businesses
        ['when', ['and', ['type', '=', 'business'], ['country', 'in', ['HR', 'SI', 'AT', 'DE']]], [
            'required', 'vat_eu',
        ]],
    ],
    'company_name' => [
        // Required for business OR if VAT number is provided
        ['when', ['or', ['type', '=', 'business'], ['vat_number', 'filled']], [
            'required', 'string', ['min', 2],
        ]],
    ],
]);
```

## Wildcard Validation

```php
$validator = new Validator([
    'items' => ['required', 'array'],
    'items.*.name' => ['required', 'string'],
    'items.*.price' => ['required', 'numeric', ['gt', 0]],
]);
```

## Error Format

Errors are returned as keys for i18n:

```php
[
    'email' => [
        [
            'key' => 'validation.required',
            'params' => ['field' => 'email'],
        ],
    ],
]
```

## Performance

| Operation | PHP Library | C Extension |
|-----------|-------------|-------------|
| 50 fields, 200 rules | 0.5-1ms | 0.05-0.1ms |
| Regex validation | Compile each time | Compile once, cache |
| Email validation | filter_var | Optimized parser |
| Memory per validation | ~50KB | ~5KB |

## Testing

```bash
make test

# Memory leak testing with Valgrind
USE_ZEND_ALLOC=0 valgrind --leak-check=full php tests/memory_test.php
```

## License

MIT
