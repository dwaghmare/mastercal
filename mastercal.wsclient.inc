<?php
/**
 * @file
 * This file includes all the WSClient functionality that the Mastercal module
 * needs to reference.
 * 
 * NOTE: the camelCase in the array keys for the API parameter is required by
 * the Master Calendar API, though not specified.
 * 
 * https://calendar.uiowa.edu/MCAPI/MCAPIService.asmx
 */

// Include the helper functions.
include_once drupal_get_path('module', 'mastercal') . '/mastercal.helper.inc';

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
function _mastercal_get_cals() {
  // Construct the API parameter for GetCalendars.
  $param = array(
    'userName' => variable_get('mastercal_default_username'),
    'password' => variable_get('mastercal_default_password'),
  );
  
  // Load the web service.
  $service = wsclient_service_load('calendar');
  
  // Get the result and load it via simplexml.
  $result = $service->GetCalendars($param);
  $respxml = simplexml_load_string($result->GetCalendarsResult);

  // Create the calendar array and return it.
  $calendars = array();
  foreach ($respxml->children() as $cal) {
    $calendars[(string)$cal->CalendarID] = (string)$cal->Name;
  }
  return $calendars;
}

/**
 * Returns array of the specified calendar.
 * 
 * @param int $cid
 */
function _mastercal_get_cal($cid) {
  // Create array parameter for the MC GetCalendar API function.
  $param = array(
    'userName' => variable_get('mastercal_default_username'),
    'password' => variable_get('mastercal_default_password'),
    'calendarId' => $cid,
  );
  
  $service = wsclient_service_load('calendar');
  $result = $service->GetCalendar($param);
  $respxml = simplexml_load_string($result->GetCalendarResult);

  $cals = array();
  
  foreach ($respxml->children() as $child) {
    foreach ($child->children() as $subchild) {
      $cals[$subchild->getName()] = (string) $subchild;
    }
  }

  return $cals;
}

/**
 * Returns array of events from the specified calendar.
 * 
 * @param $cid
 */
function _mastercal_get_events($cid) {
  // Load the calendar as an object.
  $calendar = _mastercal_load($cid);
  
  // Create the end date.
  $end_date = _mastercal_end_date($calendar->start_date, $calendar->time_range);
  
  // Construct the calendar list in the format required by the API.
  $calendar_list = array();
  $calendar_list['int'][] = $cid;
  
  // Construct the API paramter as an array for the GetEvents call.
  $param = array(
    'userName' => $calendar->username,
    'password' => $calendar->password,
    'startDate' => date('Y-m-d', $calendar->start_date),
    'endDate' => $end_date,
    'eventName' => '',
    'location' => '',
    'calendars' => $calendar_list,
    'eventTypes' => NULL,
    'udqAnswer' => NULL,
  );
  
  // Load the WSClient calendar service.
  $service = wsclient_service_load('calendar');
  
  // Get the result.
  $result = $service->GetEvents($param);
  
  // Return the result.
  return $result;
}

/**
 * Returns array of specified event.
 * 
 * @param unknown_type $param
 */
function _mastercal_get_event($param) {
  $service = wsclient_service_load('calendar');
  $result = $service->GetEvent($param);

  $xml = simplexml_load_string(utf8_encode($result->GetEventResult));

  $calenss = array();
  $count = 0;
  foreach ($xml->children() as $child) {
    $cals = array();
    foreach ($child->children() as $subchild) {
      $cals[$subchild->getName()] = (string) $subchild;
    }
    $calens[] = $cals;
    $count .= 1;
  }
  
  if (isset($calens)) {
    $calendars = $calens;
  }
  else {
    $calendars = NULL;
  }

  return $calendars;
}

/**
 * Returns array of user-defined questions (UDQ) for calendar.
 * 
 * @param int $event_id
 */
function _mastercal_get_udqs($event_id) {
  // Construct the API parameter.
  $param = array(
    'userName' => variable_get('mastercal_default_username'),
    'password' => variable_get('mastercal_default_password'),
    'eventId' => $event_id,
  );
  
  $service = wsclient_service_load('calendar');
  $result = $service->GetUdqs($param);
  
  return $result;
}