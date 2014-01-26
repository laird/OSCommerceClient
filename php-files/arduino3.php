<?php
/*
 * arduino2
 * Looks for orders in a certain status
 * 	accepts 2 parameters:
 *	- sc:	the security code
 *	- o:	the order number to be updated
 * 
 * Authors: Bo Hermannsen, dfirefire
 * 
 * v1.0 131226
 * 
 */

//configuration
define('ID_ORDER_WAITING', '1');//the order id for orders waiting to be processed
define('SECURITY_CODE', '1049a765e9c01a01086791fa553644e6');// sample: 1049a765e9c01a01086791fa553644e6 for 1234
define('SALT', '_1BHDC3_');

//security check
if(isset($_GET['sc'])){
    $to_check = SALT . $_GET['sc'] . SALT;
    if (md5($to_check)!=SECURITY_CODE){
        exit(0);
    }
} else {
    exit(0);
}
//check o paramter
if (! is_numeric($_GET['o'])){
	echo 'parameter error: o';
	exit(0);
}

//include base functions

require('includes/application_top.php'); //

// CUSTOMER INFO
$sql = sprintf("SELECT orders_id, date_purchased, delivery_name, delivery_street_address, delivery_postcode, delivery_city FROM orders
			   WHERE orders_id='%s'", (int)$_GET['o']); 
$result = tep_db_query($sql);
while($row = tep_db_fetch_array($result)){
  echo "Ordrenummer: ".$row['orders_id'];
  echo "<br>";
  echo "<br>";
  echo "Bestilt: ". $row['date_purchased'];
  echo "<br>";
  echo "<br>";
  echo $row['delivery_name'];
  echo "<br>";
  echo $row['delivery_street_address'];
  echo "<br>";
  echo $row['delivery_postcode'] . " " . $row['delivery_city'];
  echo "<br>";
  echo "-------------------------------------------------";
  echo "<br>";
}
  
// ORDER DETAILS
$sql1 = sprintf("SELECT orders_products_id, products_quantity, products_name FROM orders_products WHERE orders_id='%s'", (int)$_GET['o']);
$result1 = tep_db_query($sql1);
while($row1 = tep_db_fetch_array($result1)){
  echo $row1['products_quantity']. " * " . $row1['products_name'];

  $sql2 = sprintf("SELECT products_options_values FROM orders_products_attributes WHERE orders_id='%s' AND orders_products_id='%s'", (int)$_GET['o'], $row1['orders_products_id']);

  $result2 = tep_db_query($sql2);
  while($row2 = tep_db_fetch_array($result2)){
    echo " - ";
    echo $row2['products_options_values'];
  }
  echo "<br>";
}

// ORDER STATUS
echo "-------------------------------------------------";
echo "<br>";

$sql = sprintf("SELECT comments FROM orders_status_history WHERE orders_id='%s' AND orders_status_id=%s LIMIT 0, 1", (int)$_GET["o"], ID_ORDER_WAITING);
$result = tep_db_query($sql);
while($row = tep_db_fetch_array($result)){
  echo wordwrap($row['comments'], 37, "<br>\n");
}
echo "<br>";
echo "-------------------------------------------------";

// PAYMENT INFO
$sql = sprintf("SELECT ipaddy, ipisp FROM orders WHERE orders_id='%s'", (int)$_GET["o"]);
$result = tep_db_query($sql);
while($row = tep_db_fetch_array($result)){
  echo "<br>";
  echo "IP: ".$row['ipaddy'];
  echo "<br>";
  echo "Udbyder: ".$row['ipisp'];
  echo "<br>";
  echo "-------------------------------------------------";
  echo "<br>";
  echo "Betalingsmåde: ";
  echo "<br>";
  echo $row['payment_method'];
  echo "<br>";
}
echo 'OK';
?>
