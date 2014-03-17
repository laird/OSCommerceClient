<?php
/*
 * arduino1
 * Looks for the oldest order in a certain status, defined by ID_ORDER_WAITING
 * 	accepts 1 parameter:
 *	- sc:	the security code
 * 
 * Authors: Bo Hermannsen, dfirefire
 * 
 * v1.0 131224
 * 
 */

//configuration
define('ID_ORDER_WAITING', '2');//the order id for orders waiting to be processed
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

//include base functions

require('includes/application_top.php'); //
                  
//fetch order(s)
$sql = sprintf("SELECT orders_id, orders_status FROM orders WHERE orders_status='%s' ORDER BY orders_id  ", ID_ORDER_WAITING); //add before last " to limit to one line:  LIMIT 0, 1
$result = tep_db_query($sql);
echo "[";
while($row = tep_db_fetch_array($result)){
  echo $row['orders_id'] . " " . $row['orders_status'] . "," ;
  echo "<br />";
}
echo 'OK';
echo "]";
?> 
