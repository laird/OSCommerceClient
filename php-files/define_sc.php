<?php
/*
 * define_sc
 * Calculates the security code for a given password
 * 
 * Author: dfirefire
 * 
 * v1.0 131224
 * 
 */

//configuration
define('SALT', '_1BHDC3_');

//security code calculation
$security_code = '';
if(isset($_POST['sc'])){
    $to_calc = SALT . $_POST['sc'] . SALT;
    $security_code = md5($to_calc);
}
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Define_sc by dfirefire</title>
</head>
<body>
    <form action="<?php $_SERVER['PHP_SELF']; ?>" method="post">
        <p>Enter password to be used by Arduino as security code parameter:&nbsp;<input name="sc" type="text" value="<?php echo (isset ($_POST['sc']) ? $_POST['sc'] : ''); ?>"></input></p>
        <p><input type="submit" name="Submit" value="Calculate!"></input></p>
        <?php if($security_code != ''){
            echo '<p>Use this to define the SECURITY_CODE in the php pages: <br /><strong>' . $security_code . '</strong></p>';
            echo '<p>Use this parameter to call the pages by the Arduino: <br /><strong>sc=' . $_POST['sc'] . '</strong><br />(use ?sc=' . $_POST['sc'] . ' for the first or &sc=' . $_POST['sc'] . ' for the next parameters)</p>';
        } ?>
    </form>
</body>
</html>
 
