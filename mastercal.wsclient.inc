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

    // This prevents an undefined variable error if the MC API is down.
    $services = NULL;
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

  // Cast array to object.
  $params = (object) $param;

  // Load the WSClient calendar service.
  $service = wsclient_service_load('calendar');

  // Get the result.
  // Added error handling if the MC API is down.
  if ($service != 'false') {
    $result = $service->GetEvents($params);
  }
  else {
    drupal_set_message('Cannot connect to Master Calendar API.', 'error');
    $events = NULL;
    return $events;
  }

  // Load the response as XML.
  $xml = simplexml_load_string($result->GetEventsResult);

  // Parse the XML response from the web service using SimpleXML.
  foreach ($xml->children() as $child) {
    // Create an array to store the data.
    $temp = array();
    foreach ($child->children() as $subchild) {
      // Associate array var['Name of field'] = 'value of field'.
      $temp[$subchild->getName()] = (string) $subchild;
    }
    // Assign the $temp fields to the $cals array.
    $cals[] = $temp;
  }

  // Collapse the dates if $cals is set.
  if (isset($cals)) {
    $events = _mastercal_collapse_dates($cals);
  }
  else {
    $events = NULL;
  }

  // Return the events.
  return $events;
}

/**
 * Returns array of specified event.
 *
 * $param $cid
 *   calendar id
 * $param $did
 *   event detail id
 */
function _mastercal_get_event($cid, $did) {
  // Load the calendar as an object.
  $calendar = _mastercal_load($cid);

  // Construct the API paramter as an array for the GetEvent call.
  $param = array(
    'userName' => $calendar->username,
    'password' => $calendar->password,
    'eventDetailId' => $did,
  );

  // Load the WSClient calendar service.
  $service = wsclient_service_load('calendar');

  // Get the result.
  $result = $service->GetEvent($param);

  // Load it into XML.
  $xml = simplexml_load_string(utf8_encode($result->GetEventResult));

  // Initialize a count.
  $count = 0;

  // Iterate over each element in the XML.
  foreach ($xml->children() as $child) {
    // Initialize a temporary array.
    $temp = array();

    // Assign proper key => value pairs based on the XML element name.
    foreach ($child->children() as $subchild) {
      $temp[$subchild->getName()] = (string) $subchild;
    }

    // Assign the $temp array to $data.
    $data[] = $temp;

    // Increment the count.
    $count .= 1;
  }

  // If there is event data.
  if (isset($data)) {
    $event = $data;
  }
  else {
    $event = NULL;
  }

  // Return the event.
  return $event;
}

/**
 * Returns array of user-defined questions (UDQ) from the API for a given event.
 *
 * If no event ID is passed as an argument, this function gets the first event
 * for the calendar and uses that event as the $eid argument for the GetUdqs
 * API call.
 *
 * If no events exist, returns NULL.
 *
 * @param $cid
 */
function _mastercal_get_udqs($cid, $eid = NULL) {
  // Load the calendar as an object.
  $calendar = _mastercal_load($cid);

  // If no event ID is passed, grab the first one from the calendar.
  if ($eid == NULL) {
    // Get events for the current calendar.
    $events = _mastercal_get_events($cid);

    // Get the eid of the first event.
    foreach ($events as $id => $event) {
      $eid = $event['EventID'];
      break;
    }
  }

  // If no events exist, return NULL.
  if (empty($eid)) {
    return NULL;
  }
  else {
    // Construct the API paramter as an array for the GetUdqs call.
    $param = array(
    'userName' => $calendar->username,
    'password' => $calendar->password,
    'eventId' => $eid,
    );

    // Load the WSClient service.
    $service = wsclient_service_load('calendar');

    // Call the GetUdqs function.
    $result = $service->GetUdqs($param);

    // Load the response as XML.
    $xml = simplexml_load_string($result->GetUdqsResult);

    // Parse the XML response from the web service using SimpleXML.
    foreach ($xml->children() as $child) {
      // Create an array to store the data.
      $temp = array();
      foreach ($child->children() as $subchild) {
        // Associate array var['Name of field'] = 'value of field'.
        $temp[$subchild->getName()] = (string) $subchild;
      }
      // Assign the $temp fields to the $cals array.
      $udqs[] = $temp;
    }

    // Return the udqs.
    return $udqs;
  }
}
