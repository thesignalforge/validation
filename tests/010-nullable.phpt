--TEST--
Nullable validation rule
--SKIPIF--
<?php if (!extension_loaded('signalforge_validation')) die('skip'); ?>
--FILE--
<?php
use Signalforge\Validation\Validator;

// Nullable field
$v = new Validator([
    'middle_name' => ['nullable', 'string', ['min', 2]],
]);

// Null value should pass
var_dump($v->validate(['middle_name' => null])->valid());

// Empty string should pass
var_dump($v->validate(['middle_name' => ''])->valid());

// Missing field should pass
var_dump($v->validate([])->valid());

// Valid string should pass
var_dump($v->validate(['middle_name' => 'James'])->valid());

// Invalid string (too short) should fail
var_dump($v->validate(['middle_name' => 'J'])->failed());

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
