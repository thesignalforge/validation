--TEST--
Basic validation test
--SKIPIF--
<?php if (!extension_loaded('signalforge_validation')) die('skip'); ?>
--FILE--
<?php
use Signalforge\Validation\Validator;

$validator = new Validator([
    'email' => ['required', 'email'],
    'name' => ['required', 'string', ['min', 2]],
]);

// Valid data
$result = $validator->validate([
    'email' => 'test@example.com',
    'name' => 'John',
]);

var_dump($result->valid());
var_dump($result->failed());
var_dump($result->errors());
var_dump($result->validated());

// Invalid data
$result = $validator->validate([
    'email' => 'invalid',
    'name' => 'J',
]);

var_dump($result->valid());
var_dump($result->failed());
var_dump(count($result->errors()) > 0);
?>
--EXPECT--
bool(true)
bool(false)
array(0) {
}
array(2) {
  ["email"]=>
  string(16) "test@example.com"
  ["name"]=>
  string(4) "John"
}
bool(false)
bool(true)
bool(true)
