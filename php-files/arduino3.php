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
$sql = sprintf("SELECT orders_id, date_purchased, delivery_name, delivery_street_address, delivery_postcode, delivery_city, customers_telephone FROM orders
			   WHERE orders_id='%s'", (int)$_GET['o']); 
$result = tep_db_query($sql);
while($row = tep_db_fetch_array($result)){
  echo "Ordrenummer: "
.$row['orders_id'];
  echo "[F";
  echo "[F";
  echo "Bestilt: ".$row['date_purchased'];
  echo "[F";
  echo "[F";
  


$string = $row['delivery_name'];


$patterns = array();
$patterns[0] = 'å';
$patterns[1] = 'æ';
$patterns[2] = 'ø';
$patterns[3] = 'Å';
$patterns[4] = 'Æ';
$patterns[5] = 'Ø';

$replacements = array();
$replacements[5] = '[a';
$replacements[4] = '[e';
$replacements[3] = '[o';
$replacements[2] = '[A';
$replacements[1] = '[E';
$replacements[0] = '[O';

echo str_replace($patterns, $replacements, $string);



  echo "[F";
 
  
$string = $row['delivery_street_address'];

$patterns = array();
$patterns[0] = 'å';
$patterns[1] = 'æ';
$patterns[2] = 'ø';
$patterns[3] = 'Å';
$patterns[4] = 'Æ';
$patterns[5] = 'Ø';

$replacements = array();
$replacements[5] = '[a';
$replacements[4] = '[e';
$replacements[3] = '[o';
$replacements[2] = '[A';
$replacements[1] = '[E';
$replacements[0] = '[O';

echo str_replace($patterns, $replacements, $string);

echo "[F";
  echo $row['delivery_postcode'] . " " ;

$string = $row['delivery_city'];
$patterns = array();
$patterns[0] = 'å';
$patterns[1] = 'æ';
$patterns[2] = 'ø';
$patterns[3] = 'Å';
$patterns[4] = 'Æ';
$patterns[5] = 'Ø';

$replacements = array();
$replacements[5] = '[a';
$replacements[4] = '[e';
$replacements[3] = '[o';
$replacements[2] = '[A';
$replacements[1] = '[E';
$replacements[0] = '[O';

echo str_replace($patterns, $replacements, $string);
  echo "[F";
echo $row['customers_telephone'];
echo "[F";
  echo "------------------------------------------";
  echo "[F";
}
  
// ORDER DETAILS
$sql1 = sprintf("SELECT orders_products_id, products_quantity, products_name FROM orders_products WHERE orders_id='%s'", (int)$_GET['o']);
$result1 = tep_db_query($sql1);
while($row1 = tep_db_fetch_array($result1)){
  echo $row1['products_quantity']. " * " ;

  $string = $row1['products_name'];


$patterns = array();
$patterns[0] = 'å';
$patterns[1] = 'æ';
$patterns[2] = 'ø';
$patterns[3] = 'Å';
$patterns[4] = 'Æ';
$patterns[5] = 'Ø';

$replacements = array();
$replacements[5] = '[a';
$replacements[4] = '[e';
$replacements[3] = '[o';
$replacements[2] = '[A';
$replacements[1] = '[E';
$replacements[0] = '[O';

echo str_replace($patterns, $replacements, $string);
  
  $sql2 = sprintf("SELECT products_options_values FROM orders_products_attributes WHERE orders_id='%s' AND orders_products_id='%s'", (int)$_GET['o'], $row1['orders_products_id']);

  $result2 = tep_db_query($sql2);
  while($row2 = tep_db_fetch_array($result2)){
    echo " - ";
    echo $row2['products_options_values'];
  


};

echo "[F";
}





// ORDER TOTALS
echo "------------------------------------------";


$sql = sprintf("SELECT title, text FROM orders_total WHERE orders_id='%s'", (int)$_GET["o"]);
$result = tep_db_query($sql);
while($row = tep_db_fetch_array($result)){
echo "[F";
  
  echo $row['title'];
  echo " ";
  
  
$string = $row['text'];
$patterns = array();
$patterns[0] = '/<strong>/';
$patterns[1] = '<<\/strong>>';
$patterns[2] = '<<span class="productSpecialPrice">>';
$patterns[3] = '<<\/span>>';  
$patterns[4] = '/<b>/';
$patterns[5] = '<<\/b>>';

  
$replacements = array();
$replacements[5] = '[B';
$replacements[4] = '[b';
$replacements[3] = '';
$replacements[2] = '';
$replacements[1] = '[B';
$replacements[0] = '[b';
echo preg_replace($patterns, $replacements, $string);

  
  }
echo "[F";
  
// ORDER STATUS
echo "------------------------------------------";

echo "[F";
$sql = sprintf("SELECT comments FROM orders_status_history WHERE orders_id='%s' AND orders_status_id=%s ", (int)$_GET["o"], ID_ORDER_WAITING);
$result = tep_db_query($sql);
while($row = tep_db_fetch_array($result)){
  

$string = $row['comments'];
$patterns = array();
$patterns[0] = 'å';
$patterns[1] = 'æ';
$patterns[2] = 'ø';
$patterns[3] = 'Å';
$patterns[4] = 'Æ';
$patterns[5] = 'Ø';

$patterns[6] = '´';
$patterns[7] = '`';
$patterns[8] = '|';
$patterns[9] = '^';
$patterns[10] = '¨';
$patterns[11] = '~';
$patterns[12] = '_';

$replacements = array();
$replacements[12] = '[a';
$replacements[11] = '[e';
$replacements[10] = '[o';
$replacements[9] = '[A';
$replacements[8] = '[E';
$replacements[7] = '[O';

$replacements[6] = '';
$replacements[5] = '';
$replacements[4] = '';
$replacements[3] = '';
$replacements[2] = '';
$replacements[1] = '';
$replacements[0] = '';

echo str_replace($patterns, $replacements, $string);
  echo "[F";
}
echo "[F";
echo "------------------------------------------";
echo "[F";
// PAYMENT INFO
$sql = sprintf("SELECT ipaddy, ipisp FROM orders WHERE orders_id='%s'", (int)$_GET["o"]);
$result = tep_db_query($sql);
while($row = tep_db_fetch_array($result)){
  
  echo "IP: ".$row['ipaddy'];
  echo "[F";
  echo "Udbyder: ".$row['ipisp'];
  echo "[F";
  echo "------------------------------------------";
}
$sql = sprintf("SELECT payment_method FROM orders WHERE orders_id='%s'", (int)$_GET["o"]);
$result = tep_db_query($sql);
while($row = tep_db_fetch_array($result)){
echo "[F";
  
  echo "[B"."[D"."Betalingsm[ade: ";
  
  echo $row['payment_method']."[b"."[d";
  
}




?>
