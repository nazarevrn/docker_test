--TEST--
PDO Common: transactions
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded('pdo')) die('skip');
$dir = getenv('REDIR_TEST_DIR');
if (false == $dir) die('skip no driver');
require_once $dir . 'pdo_test.inc';
PDOTest::skip();

$db = PDOTest::factory();
try {
  $db->beginTransaction();
} catch (PDOException $e) {
  die('skip no working transactions: ' . $e->getMessage());
}

if ($db->getAttribute(PDO::ATTR_DRIVER_NAME) == 'mysql') {
	if (false === PDOTest::detect_transactional_mysql_engine($db)) {
		die('skip your mysql configuration does not support working transactions');
	}
}
?>
--FILE--
<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

if ($db->getAttribute(PDO::ATTR_DRIVER_NAME) == 'mysql') {
	$suf = ' Type=' . PDOTest::detect_transactional_mysql_engine($db);
} else {
	$suf = '';
}

$db->exec('CREATE TABLE test(id INT NOT NULL PRIMARY KEY, val VARCHAR(10))'.$suf);
$db->exec("INSERT INTO test VALUES(1, 'A')"); 
$db->exec("INSERT INTO test VALUES(2, 'B')"); 
$db->exec("INSERT INTO test VALUES(3, 'C')");
$delete = $db->prepare('DELETE FROM test');

function countRows($action) {
    global $db;
	$select = $db->prepare('SELECT COUNT(*) FROM test');
	$select->execute();
    $res = $select->fetchColumn();
    return "Counted $res rows after $action.\n";
}

echo countRows('insert');

$db->beginTransaction();
$delete->execute();
echo countRows('delete');
$db->rollBack();

echo countRows('rollback');

?>
--EXPECT--
Counted 3 rows after insert.
Counted 0 rows after delete.
Counted 3 rows after rollback.
