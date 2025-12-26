<?php
/**
 * Signalforge Validation Extension - Basic Usage Example
 */

use Signalforge\Validation\Validator;
use Signalforge\Validation\ValidationResult;

// Ensure extension is loaded
if (!extension_loaded('signalforge_validation')) {
    die("Error: signalforge_validation extension is not loaded.\n");
}

echo "=== Signalforge Validation Extension Demo ===\n\n";

// 1. Basic validation
echo "1. Basic Validation\n";
echo str_repeat("-", 40) . "\n";

$validator = new Validator([
    'email' => ['required', 'email'],
    'name' => ['required', 'string', ['min', 2], ['max', 100]],
    'age' => ['nullable', 'integer', ['gte', 18]],
]);

$result = $validator->validate([
    'email' => 'user@example.com',
    'name' => 'John Doe',
    'age' => 25,
]);

echo "Valid: " . ($result->valid() ? 'Yes' : 'No') . "\n";
echo "Validated data: " . json_encode($result->validated()) . "\n\n";

// 2. Validation with errors
echo "2. Validation with Errors\n";
echo str_repeat("-", 40) . "\n";

$result = $validator->validate([
    'email' => 'invalid-email',
    'name' => 'A',
    'age' => 16,
]);

echo "Valid: " . ($result->valid() ? 'Yes' : 'No') . "\n";
echo "Errors:\n";
foreach ($result->errors() as $field => $errors) {
    echo "  $field:\n";
    foreach ($errors as $error) {
        echo "    - {$error['key']}\n";
    }
}
echo "\n";

// 3. Static factory method
echo "3. Static Factory Method\n";
echo str_repeat("-", 40) . "\n";

$validator = Validator::make(
    ['email' => 'test@example.com'],
    ['email' => ['required', 'email']]
);
$result = $validator->validate(['email' => 'test@example.com']);
echo "Valid: " . ($result->valid() ? 'Yes' : 'No') . "\n\n";

// 4. Wildcard validation
echo "4. Wildcard Validation\n";
echo str_repeat("-", 40) . "\n";

$validator = new Validator([
    'items' => ['required', 'array', ['min', 1]],
    'items.*.name' => ['required', 'string', ['max', 100]],
    'items.*.price' => ['required', 'numeric', ['gt', 0]],
]);

$result = $validator->validate([
    'items' => [
        ['name' => 'Product A', 'price' => 99.99],
        ['name' => 'Product B', 'price' => 49.50],
        ['name' => 'Product C', 'price' => 149.00],
    ],
]);

echo "Valid: " . ($result->valid() ? 'Yes' : 'No') . "\n";
echo "Items validated successfully!\n\n";

// 5. Conditional validation
echo "5. Conditional Validation\n";
echo str_repeat("-", 40) . "\n";

$validator = new Validator([
    'type' => ['required', ['in', ['personal', 'business']]],
    'company_name' => [
        ['when', ['type', '=', 'business'], [
            'required', 'string', ['min', 2],
        ]],
    ],
    'vat_number' => [
        ['when', ['type', '=', 'business'], [
            'required', 'string',
        ]],
    ],
]);

// Business type - requires company details
$result = $validator->validate([
    'type' => 'business',
    'company_name' => 'Acme Inc',
    'vat_number' => 'HR12345678901',
]);
echo "Business registration: " . ($result->valid() ? 'Valid' : 'Invalid') . "\n";

// Personal type - no company details needed
$result = $validator->validate([
    'type' => 'personal',
]);
echo "Personal registration: " . ($result->valid() ? 'Valid' : 'Invalid') . "\n\n";

// 6. Regional validation (Croatian OIB)
echo "6. Regional Validation (Croatian OIB)\n";
echo str_repeat("-", 40) . "\n";

$validator = new Validator([
    'oib' => ['required', 'oib'],
]);

$result = $validator->validate(['oib' => '12345678903']);
echo "OIB 12345678903: " . ($result->valid() ? 'Valid' : 'Invalid') . "\n";

$result = $validator->validate(['oib' => '00000000000']);
echo "OIB 00000000000: " . ($result->valid() ? 'Valid' : 'Invalid') . "\n\n";

// 7. Password confirmation
echo "7. Password Confirmation\n";
echo str_repeat("-", 40) . "\n";

$validator = new Validator([
    'password' => ['required', 'string', ['min', 8], 'confirmed'],
]);

$result = $validator->validate([
    'password' => 'SecurePass123',
    'password_confirmation' => 'SecurePass123',
]);
echo "Passwords match: " . ($result->valid() ? 'Yes' : 'No') . "\n";

$result = $validator->validate([
    'password' => 'SecurePass123',
    'password_confirmation' => 'DifferentPass',
]);
echo "Mismatched passwords: " . ($result->failed() ? 'Correctly rejected' : 'Error') . "\n\n";

echo "=== Demo Complete ===\n";
