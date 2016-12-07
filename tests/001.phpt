--TEST--
PhpLzf::apiVersion
--SKIPIF--
<?php if (!extension_loaded("phplzf")) die("skip"); ?>
--FILE--
<?php
var_dump(phplzf_api_version());
?>
--CLEAN--
===DONE===
--EXPECT--
string(5) "0.1.1"
===DONE===
