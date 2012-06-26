<?php
/**
 * @file
 * This file includes all the WSClient functionality that the Mastercal module
 * needs to reference.
 */

/**
 * Implements hook_default_wsclient_service().
 * 
 * Initialize the calendar service that uses the MC API.
 */
function mastercal_default_wsclient_service() {

  // Geocoder.us service (SOAP)
  $service = new WSClientServiceDescription();
  $service->name = 'calendar';
  $service->label = 'MasterCal';
  $service->url = 'https://calendar.uiowa.edu/MCAPI/MCAPIService.asmx?wsdl';
  $service->type = 'soap';
  $operation = array();
  $operation['label'] = 'GetEventTypes';
  $operation['name'] = 'GetEventTypes';
  $operation['parameter']['userName'] = array('type' => 'text', 'default value' => 'Arts_Iowa_API');
  $operation['parameter']['passWord'] = array('type' => 'text', 'default value' => 'Ap3dn9!nez');
  $operation['result'] = array('type' => 'list\u003cGetEventTypesResponse\u003e', 'label' => 'GetEventTypesResponse');
  $service->operations['GetEventTypes'] = $operation;
  try {
    $service->endpoint()->initializeMetaData();
    $services[$service->name] = $service;
  }
  catch (WSClientException $e) {
    watchdog('wsclient', $e->__toString());
  }
  
  return $services;
}

function mastercal_getCals($param) {
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

function mastercal_getCal($param) {
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

function mastercal_getEvents($param) {
   
  //print_r($cal_list);
  //the Camel Case is required and not specified in the API but can be found by examining the request example on the API site /MCAPI/MCAPIService.asmx

  //dpm($param);
  $service = wsclient_service_load('calendar');
  $result = $service->GetEvents($param);
  return $result;
}
function mastercal_getEvent($param) {
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