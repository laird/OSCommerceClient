<?php
/*
 * update_order
 * updates an order
 * 	accepts 3 parameters:
 *	- sc:	the security code
 *	- o:	the order number to be updated
 *	- s:	the status command, either "processing" or "finished"
 * 
 * Authors: dfirefire
 * 
 * v1.0 131226
 * 
 */

//include base functions

require('includes/application_top.php'); //

//configuration
define('ID_ORDER_WAITING', '1');//the order id for orders waiting to be processed
define('ID_ORDER_PROCESSING', '2');//the order is busy
define('ID_ORDER_FINISHED', '3');//the order is finished
define('SECURITY_CODE', '1049a765e9c01a01086791fa553644e6');// sample: 1049a765e9c01a01086791fa553644e6 for 1234
define('SALT', '_1BHDC3_');//extra security on the md5
define('NOTIFY_CUSTOMER_ARDUINO', false);//send an e-mail to the customer upon status change?
define('NOTIFY_CUSTOMER_COMMENT', 'Automated update');//the comment that will be stored in order_status_history and sent in the mail
define('PATH_TO_ADMIN_LANG_ORDERS','C:/wamp/www/butikejer/includes/languages/' . $language . '/orders.php');//needed for e-mail contents

//get the language definitions
require(PATH_TO_ADMIN_LANG_ORDERS);

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
if(!isset($_GET['o'])||(! is_numeric($_GET['o']))){
	echo 'parameter error: o';
	exit(0);
}

//check s parameter
if(! isset($_GET['s'])){
	echo 'parameter error: s';
	exit(0);
}

//start processing
$status_goal = ID_ORDER_WAITING;
switch($_GET['s']){
	case 'processing':
		$status_goal = ID_ORDER_PROCESSING;
		break;
	case 'finished':
		$status_goal = ID_ORDER_FINISHED;
		break;
	default:
		echo 'parameter error: wrong status';
		exit(0);
}

//update table orders
$sql_data_array = array('orders_status' => $status_goal,
						'last_modified' => 'now()');
$parameters = sprintf(" orders_id=%s", (int)$_GET['o']);
tep_db_perform(TABLE_ORDERS, $sql_data_array, 'update', $parameters);

//send mail
$notify_arduino_ok = 0;
if(NOTIFY_CUSTOMER_ARDUINO){
	//get the statuses to inform customer
	$orders_statuses = array();
  	$orders_status_array = array();
	$orders_status_sql = sprintf("select orders_status_id, orders_status_name from %s where language_id = '%s'", TABLE_ORDERS_STATUS, (int)$languages_id);
  	$orders_status_query = tep_db_query($orders_status_sql);
  	while ($orders_status = tep_db_fetch_array($orders_status_query)) {
    	$orders_statuses[] = array('id' => $orders_status['orders_status_id'],
                               'text' => $orders_status['orders_status_name']);
    	$orders_status_array[$orders_status['orders_status_id']] = $orders_status['orders_status_name'];
  	}
	//get the order information to notify the customer
	$check_status__sql = sprintf("select customers_name, customers_email_address, orders_status, date_purchased from %s where orders_id = '%s'", TABLE_ORDERS, (int)$_GET['o']);
	$check_status_query = tep_db_query($check_status__sql);
	$check_status = tep_db_fetch_array($check_status_query);

	$email = STORE_NAME . "\n" . EMAIL_SEPARATOR . "\n" . EMAIL_TEXT_ORDER_NUMBER . ' ' . (int)$_GET['o'] . "\n" 
		. EMAIL_TEXT_INVOICE_URL . ' ' . tep_href_link(FILENAME_ACCOUNT_HISTORY_INFO, 'order_id=' . (int)$_GET['o'], 'SSL') . "\n" 
		. EMAIL_TEXT_DATE_ORDERED . ' ' . tep_date_long($check_status['date_purchased']) . "\n\n" 
		. $notify_comments . sprintf(EMAIL_TEXT_STATUS_UPDATE, $orders_status_array[$status_goal]);
	tep_mail($check_status['customers_name'], $check_status['customers_email_address'], EMAIL_TEXT_SUBJECT, $email, STORE_OWNER, STORE_OWNER_EMAIL_ADDRESS);
	$notify_arduino_ok = 1;
}

//update table orders_status_history
$sql_data_array = array('orders_id' => (int)$_GET['o'],
						'orders_status_id' => $status_goal,
						'date_added' => 'now()',
						'customer_notified' => $notify_arduino_ok,
						'comments' => NOTIFY_CUSTOMER_COMMENT
						);
tep_db_perform(TABLE_ORDERS_STATUS_HISTORY, $sql_data_array);

echo 'OK';
?>
