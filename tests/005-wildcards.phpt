--TEST--
Wildcard validation rules
--SKIPIF--
<?php if (!extension_loaded('signalforge_validation')) die('skip'); ?>
--FILE--
<?php
use Signalforge\Validation\Validator;

// Wildcard validation for array items
$v = new Validator([
    'items' => ['required', 'array'],
    'items.*.name' => ['required', 'string', ['min', 2]],
    'items.*.price' => ['required', 'numeric', ['gt', 0]],
]);

// Valid items
$result = $v->validate([
    'items' => [
        ['name' => 'Product 1', 'price' => 99.99],
        ['name' => 'Product 2', 'price' => 49.99],
    ]
]);
var_dump($result->valid());

// Invalid item name
$result = $v->validate([
    'items' => [
        ['name' => 'A', 'price' => 99.99],  // Name too short
    ]
]);
var_dump($result->failed());

// Invalid item price
$result = $v->validate([
    'items' => [
        ['name' => 'Product', 'price' => 0],  // Price not > 0
    ]
]);
var_dump($result->failed());

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
OK
