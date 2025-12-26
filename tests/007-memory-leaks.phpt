--TEST--
Memory leak test - repeated validation
--SKIPIF--
<?php if (!extension_loaded('signalforge_validation')) die('skip'); ?>
--FILE--
<?php
use Signalforge\Validation\Validator;

$validator = new Validator([
    'email' => ['required', 'email'],
    'items' => ['required', 'array'],
    'items.*.name' => ['required', 'string', ['max', 100]],
]);

// Run many times to detect leaks
for ($i = 0; $i < 10000; $i++) {
    $result = $validator->validate([
        'email' => 'test@example.com',
        'items' => [
            ['name' => 'Item 1'],
            ['name' => 'Item 2'],
        ],
    ]);
}

echo "OK\n";
?>
--EXPECT--
OK
