<?php

/**
 * Custom exception class to enhance default PHP exceptions.
 */
require_once('nusoap/nusoap.php');
class mastercalSOAPexception extends Exception {
  /**
   * @param $msg
   *   The exception message containing placeholder as t().
   * @param $args
   *   Replacement arguments such as for t().
   */
  function __construct($msg, $args = array()) {
    $message = t($msg, $args);
    parent::__construct($message);
  }
}


/**
 * A soap client class specific to MasterCalendar
 */
class mastercalSOAP {

  protected $client;
  protected $url;
  protected $un;
  protected $pw;
  protected $param;

  public function __construct($options = Null) {
    $this->un = 'emsapi@uiowa.edu';
    $this->pw = 'emsapi2011';
    $this->url = 'http://itsnt917.iowa.uiowa.edu/MCAPI/MCAPIService.asmx?wsdl';
    
  }
  public function client() {
    if (!isset($this->client)) {
      $options['exceptions'] = TRUE;

      // The url has to point to a valid WSDL file.
      $wsdl = @file_get_contents($this->url);
      if ($wsdl) {
        $this->client = new nusoap_client($this->url, true);
        $err = $this->client->getError();
        if ($err) {
          // Display the error
          drupal_set_message('<h2>Constructor error</h2><pre>' . $err . '</pre>', 'Error');
        }
      }
      else {
        throw new mastercalSOAPexception('Unable to fetch WSDL file from %wsdl', array('%wsdl' => $this->url));
      }
    }
    return $this->client;
  }



  /**
   * Calls the SOAP service.
   *
   * @param string $operation
   *   The name of the operation to execute.
   * @param array $arguments
   *   Arguments to pass to the service with this operation.
   */
  public function call($operation, $arguments) {
    $client = $this->client();
    $response = $client->call($operation, $arguments);
    //print_r($operation);
    //print_r($arguments);
    // Check for a fault
    if ($client->fault) {
        echo '<p><b>Fault: ';
        print_r($response);
        
        echo '</b></p>';
    } else {
        // Check for errors
        $err = $client->getError();
        if ($err) {
            // Display the error
            echo '<p><b>Error: ' . $err . '</b></p>';
        } else {
            // Display the result
            return $response;
        }
    }  

  }
  

  
  public function GetCal($calID) {
    $param = array(
      'userName' => $this->un,
      'password' => $this->pw,
      'calendarId' => $calID,
    );

    $result = $this->call('GetCalendar', $param);
    //print_r($result);
    $respxml = simplexml_load_string($result['GetCalendarResult']);
    
    //print_r($respxml);
    $cals = array();
    foreach($respxml->children() as $child) {
      //print_r($child->calendar);
      foreach($child->children() as $subchild) {
        $cals[$subchild->getName()] = (string) $subchild;
        //print_r("\n".$subchild->getName().": ".$subchild."");
      }
    }

    return $cals;
  }
  
  public function GetCals() {
    $param = array(
      'userName' => $this->un,
      'password' => $this->pw,
    );

    $result = $this->call('GetCalendars', $param);
    //print_r($result);
    $respxml = simplexml_load_string($result['GetCalendarsResult']);
    
    //print_r($respxml);
    $calendars = array();
    foreach ($respxml->children() as $cal) {
      $calendars[(string)$cal->CalendarID] = (string)$cal->Name;
    }
    
    $calendars = $this->filterCals($calendars);
    //print_r($calendars);
    return $calendars;
  }
  
  public function GetEvents() {
    $cal_ids = mastercal_load();
    $cal_list = array();
    //make sure the lsit isnt empty
    if (isset($cal_ids)) { 
      //for the soap handler to identify your array as an integer array you have to make an associative array where 'int' => array(12,32,43)
      foreach ($cal_ids as $item) {
        $cal_list['int'][] = $item->cid;
      }
    }
    //print_r($cal_list);
    //the Camel Case is required and not specified in the API but can be found by examining the request example on the API site /MCAPI/MCAPIService.asmx
    $param['userName'] = $this->un;
    $param['password'] = $this->pw; 
    $param['startDate'] = $this->firstOfMonth();
    $param['endDate'] = $this->lastOfMonth();
    $param['location'] = '';
    $param['eventName'] = '';
    $param['calendars'] = $cal_list;
    $param['eventTypes'] = Null;
    
    //print_r($param);
    $result = $this->call('GetEvents', $param);
    //print_r($result);
    $xml = simplexml_load_string(utf8_encode($result['GetEventsResult']));
    $calenss = array();
    $count = 0;
    //parse the xml response from the web service using SimpleXML
    foreach($xml->children() as $child) {
      //create an array to store the data
      $cals = array();
      foreach($child->children() as $subchild) {
        //associate array var['Name of field'] = 'value of field'
        $cals[$subchild->getName()] = (string) $subchild;
      }
      $calens[] = $cals;
      $count .= 1;
    }
    //print_r($calens);


    //print_r($respxml);
    if (isset($calens)) {
      $calendars = $calens;
    }else {
      $calendars = Null;
    }

    //print_r($calendars);
    return $calendars;
  }
  //not being used so far but will be needed in order to get images from events
  public function GetEvent() {
    $param['userName'] = $this->un;
    $param['password'] = $this->pw; 

    
    $param['eventDetailId'] = 20841;

    
    //print_r($param);
    $result = $this->call('GetEvent', $param);
    //print_r($result);
    $xml = simplexml_load_string(utf8_encode($result['GetEventResult']));
    $calenss = array();
    $count = 0;
    foreach($xml->children() as $child) {
      //print_r("\n---------------------------");
      $cals = array();
      foreach($child->children() as $subchild) {
        $cals[$subchild->getName()] = (string) $subchild;
        //print_r("\n".$subchild->getName().": ".$subchild."");
      }
      $calens[] = $cals;
      $count .= 1;
    }
    //print_r($calens);
    //$respxml = $this->MakeXml(utf8_encode($result['GetEventsResult']));
    //print_r(variable_get('mastercal_name', 'poop'));
    //print_r($respxml);
    if (isset($calens)) {
      $calendars = $calens;
    }else {
      $calendars = Null;
    }

    //print_r($calendars);
    return $calendars;
  }
  
  
  /**
   * Helper functions
   */
   
   //currently only filters out private calednars but may be expanded at some point
  function filterCals($allCals) {
    $filtered = array();
    foreach($allCals as $calid => $calname) {
      $calData = $this->GetCal($calid);
      //print_r($calData);
      if ($calData['IsPrivate'] != 'true') {
        $filtered[$calid] =$calname;        
      }
    }
    //print_r($filtered);
    return $filtered;
  }
  
  //currently not being used
  function MakeXml($soaprsp) {
    return simplexml_load_string($soaprsp)->asXML();
  }
  
  //returns the date of the first of the month 'yyyy-mm-dd'
  function firstOfMonth() {
    return date("Y-m-d", strtotime(date('m').'/01/'.date('Y').' 00:00:00'));
  }

  //date of the last day of the month
  function lastOfMonth() {
    return date("Y-m-d", strtotime('-1 second',strtotime('+1 month',strtotime(date('m').'/01/'.date('Y').' 00:00:00'))));
  }

}