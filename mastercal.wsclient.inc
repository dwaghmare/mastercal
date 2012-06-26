<?php
/**
 * @file
 * This file includes all the WSClient functionality that the Mastercal module
 * needs to reference.
 */

// Obscure the username and password.
$un = base64_decode('QXJ0c19Jb3dhX0FQSQ==');
$pw = base64_decode('QXAzZG45IW5leg==');

// Declare a global variable to be used in the WSClient function calls.
$_mastercal_param = array(
  'userName' => $un,
  'password' => $pw,
  'startDate' => firstOfMonth(),
  'endDate' => lastOfMonth(),
  'location' => '',
  'eventName' => '',
  'calendars' => mastercal_load(),
  'eventTypes' => NULL,
  'udqAnswer' => NULL,
);

/**
 * Implements hook_default_wsclient_service().
 * 
 * Initialize the calendar service that uses the MC API.
 */
function mastercal_default_wsclient_service() {

  // UIowa master calendar service (SOAP).
  $service = new WSClientServiceDescription();
  $service->name = 'calendar';
  $service->label = 'MasterCal';
  $service->url = 'https://calendar.uiowa.edu/MCAPI/MCAPIService.asmx?wsdl';
  $service->type = 'soap';
  
  try {
    $service->endpoint()->initializeMetaData();
    $services[$service->name] = $service;
  }
  catch (WSClientException $e) {
    watchdog('wsclient', $e->__toString());
  }
  
  return $services;
}

/**
 * Returns an array of calendars available on the UIowa Master Calendar.
 * 
 * @param array $param
 */
function mastercal_get_cals($param) {
  $service = wsclient_service_load('calendar');
  $result = $service->GetCalendars($param);
  $respxml = simplexml_load_string($result->GetCalendarsResult);

  //print_r($respxml);
  $calendars = array();
  foreach ($respxml->children() as $cal) {
    $calendars[(string)$cal->CalendarID] = (string)$cal->Name;
  }
  return $calendars;
}

/**
 * Returns array of specified calendar.
 * 
 * @param unknown_type $param
 */
function mastercal_get_cal($param) {
  $service = wsclient_service_load('calendar');
  $result = $service->GetCalendar($param);
  $respxml = simplexml_load_string($result->GetCalendarResult);

  //print_r($respxml);
  $cals = array();
  foreach ($respxml->children() as $child) {
    //print_r($child->calendar);
    foreach ($child->children() as $subchild) {
      $cals[$subchild->getName()] = (string) $subchild;
      //print_r("\n".$subchild->getName().": ".$subchild."");
    }
  }

  return $cals;
}

/**
 * Returns array of events from specified calendar.
 * 
 * @param unknown_type $param
 */
function mastercal_get_events($param) {
   
  //print_r($cal_list);
  //the Camel Case is required and not specified in the API but can be found by examining the request example on the API site /MCAPI/MCAPIService.asmx

  //dpm($param);
  $service = wsclient_service_load('calendar');
  $result = $service->GetEvents($param);
  return $result;
}

/**
 * Returns array of specified event.
 * 
 * @param unknown_type $param
 */
function mastercal_get_event($param) {
  $service = wsclient_service_load('calendar');
  $result = $service->GetEvent($param);
  //$result = $this->call('GetEvent', $param);

  //print_r($result);
  $xml = simplexml_load_string(utf8_encode($result->GetEventResult));
  //print_r($result);
  $calenss = array();
  $count = 0;
  foreach ($xml->children() as $child) {
    //print_r("\n---------------------------");
    $cals = array();
    foreach ($child->children() as $subchild) {
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
  }
  else {
    $calendars = Null;
  }

  //print_r($calendars);
  return $calendars;
}