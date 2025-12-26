--TEST--
Comparison validation rules
--SKIPIF--
<?php if (!extension_loaded('signalforge_validation')) die('skip'); ?>
--FILE--
<?php
use Signalforge\Validation\Validator;

// In
$v = new Validator(['status' => [['in', ['active', 'inactive', 'pending']]]]);
var_dump($v->validate(['status' => 'active'])->valid());
var_dump($v->validate(['status' => 'unknown'])->failed());

// Not In
$v = new Validator(['status' => [['not_in', ['banned', 'deleted']]]]);
var_dump($v->validate(['status' => 'active'])->valid());
var_dump($v->validate(['status' => 'banned'])->failed());

// Same
$v = new Validator([
    'password' => ['required', 'string'],
    'password_confirm' => ['required', ['same', 'password']],
]);
var_dump($v->validate(['password' => 'secret', 'password_confirm' => 'secret'])->valid());
var_dump($v->validate(['password' => 'secret', 'password_confirm' => 'different'])->failed());

// Different
$v = new Validator([
    'new_password' => ['required', ['different', 'old_password']],
]);
var_dump($v->validate(['old_password' => 'old', 'new_password' => 'new'])->valid());
var_dump($v->validate(['old_password' => 'same', 'new_password' => 'same'])->failed());

// Confirmed
$v = new Validator([
    'password' => ['required', 'confirmed'],
]);
var_dump($v->validate(['password' => 'secret', 'password_confirmation' => 'secret'])->valid());
var_dump($v->validate(['password' => 'secret', 'password_confirmation' => 'wrong'])->failed());

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
