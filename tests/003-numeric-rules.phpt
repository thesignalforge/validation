--TEST--
Numeric validation rules
--SKIPIF--
<?php if (!extension_loaded('signalforge_validation')) die('skip'); ?>
--FILE--
<?php
use Signalforge\Validation\Validator;

// Integer
$v = new Validator(['age' => ['integer']]);
var_dump($v->validate(['age' => 25])->valid());
var_dump($v->validate(['age' => '25'])->valid());
var_dump($v->validate(['age' => 'abc'])->failed());

// Numeric
$v = new Validator(['price' => ['numeric']]);
var_dump($v->validate(['price' => 99.99])->valid());
var_dump($v->validate(['price' => '99.99'])->valid());
var_dump($v->validate(['price' => 'abc'])->failed());

// GT/GTE/LT/LTE
$v = new Validator(['age' => ['integer', ['gt', 18]]]);
var_dump($v->validate(['age' => 19])->valid());
var_dump($v->validate(['age' => 18])->failed());

$v = new Validator(['age' => ['integer', ['gte', 18]]]);
var_dump($v->validate(['age' => 18])->valid());
var_dump($v->validate(['age' => 17])->failed());

$v = new Validator(['age' => ['integer', ['lt', 65]]]);
var_dump($v->validate(['age' => 64])->valid());
var_dump($v->validate(['age' => 65])->failed());

$v = new Validator(['age' => ['integer', ['lte', 65]]]);
var_dump($v->validate(['age' => 65])->valid());
var_dump($v->validate(['age' => 66])->failed());

// Between for numeric
$v = new Validator(['age' => ['integer', ['between', 18, 65]]]);
var_dump($v->validate(['age' => 30])->valid());
var_dump($v->validate(['age' => 17])->failed());
var_dump($v->validate(['age' => 66])->failed());

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
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
