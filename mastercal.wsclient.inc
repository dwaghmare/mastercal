<?php
/**
 * @file
 * This file includes all the WSClient functionality that the Mastercal module
 * needs to reference.
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
function _mastercal_get_cals($param) {
  $service = wsclient_service_load('calendar');
  $result = $service->GetCalendars($param);
  $respxml = simplexml_load_string($result->GetCalendarsResult);

  $calendars = array();
  foreach ($respxml->children() as $cal) {
    $calendars[(string)$cal->CalendarID] = (string)$cal->Name;
  }
  return $calendars;
}

/**
 * Returns array of the specified calendar.
 * 
 * @param int $calendar_id
 */
function _mastercal_get_cal($calendar_id) {
  // Create array parameter for the MC GetCalendar API function.
  $param = array(
    'userName' => variable_get('mastercal_default_username'),
    'password' => variable_get('mastercal_default_password'),
    'calendarId' => $calendar_id,
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
 * @param array $param
 */
function _mastercal_get_events($param) {
  // The Camel Case is required but not specified in the API. It can be found
  // by examining the request example on the API site /MCAPI/MCAPIService.asmx.
  $service = wsclient_service_load('calendar');
  $result = $service->GetEvents($param);
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