<?php
/**
 * @file
 * This file includes all the helper functions for the mastercal module.
 */

/**
 * Helper function to find first day of the current month.
 * 
 * @return date
 */
function _mastercal_first_of_month() {
  return date("Y-m-d", strtotime(date('m') . '/01/' . date('Y') . ' 00:00:00'));
}

/**
 * Helper function to find last day of the current month.
 * 
 * @return date
 */
function _mastercal_last_of_month() {
  return date("Y-m-d", strtotime('-1 second', strtotime('+1 month', strtotime(date('m') . '/01/' . date('Y') . ' 00:00:00'))));
}

/**
 * Helper function to return an end date give a start date and a range.
 * 
 * @param date $start_date
 * @param int $range
 *   In number of months.
 */
function _mastercal_end_date($start_date, $range) {
  
}

/**
 * Helper function to collapse dates.
 *
 * @param array $events
 */
function _mastercal_collapse_dates($events) {
  $collapsed = array();
  $viewed = array();
  foreach ($events as $event) {
    $cid = $event['EventID'];
    if (isset($viewed[$cid])) {
      $collapsed[$cid]['EventDate'][] = array(
        'Date' => $event['EventDate'],
        'Start' => $event['TimeEventStart'],
        'End' => $event['TimeEventEnd']
      );
    }
    else {
      $viewed[$cid] = True;
      $collapsed[$cid] = $event;
      $collapsed[$cid]['EventDate'] = array();
      $collapsed[$cid]['EventDate'][] = array('Date' => $event['EventDate'], 'Start' => $event['TimeEventStart'], 'End' => $event['TimeEventEnd']);
    }
  }
  return $collapsed;
}

/**
 * Helper function to create taxonomy list.
 *
 * @param array $list
 */
function _mastercal_create_terms($list) {
  $jobs = array();

  // Grabs the list of vocabularies.
  $vocabularies = taxonomy_get_vocabularies();
  foreach ($vocabularies as $voc) {
    if ($voc->machine_name == 'event_types') {
      $vid = $voc->vid;
    }
  }
  // Grabs the terms from the chosen vocabulary.
  $existing = taxonomy_get_tree($vid);

  $loaded_terms = array();
  foreach ($existing as $job_rank) {
    $loaded_terms[] = $job_rank->name;
  }
  foreach ($list as $entry) {
    if (is_array($entry)) {
      if (!isset($jobs[$entry['title']])) {
        $jobs[$entry['title']] = False;
      }
    }
    else {
      if (!isset($jobs[$entry])) {
        $jobs[$entry] = False;
      }

    }
  }

  if (!empty($loaded_terms)) {
    foreach ($jobs as $new_term => $val) {

      if (!in_array($new_term, $loaded_terms)) {
        taxonomy_term_save((object) array(
          'name' => t($new_term),
          'vid' => $vid,
        ));
      }
    }
  }
  else {
    foreach ($jobs as $new_term => $val) {
      taxonomy_term_save((object) array(
        'name' => t($new_term),
        'vid' => $vid,
      ));
    }
  }


}

/**
 * Helper function to create taxonomy term.
 *
 * @param string $event_type
 */
function _mastercal_create_term($event_type) {

  // Grabs the list of vocabularies.
  $vocabularies = taxonomy_get_vocabularies();
  foreach ($vocabularies as $voc) {
    if ($voc->machine_name == 'event_types') {
      $vid = $voc->vid;
    }
  }
  // Grabs the terms from the chosen vocabulary.
  if (!empty($vid)) {
    $new = array('vid' => $vid, 'name' => $event_type);
    $terms = taxonomy_get_term_by_name($new['name']);
    if (!empty($terms)) {
      if (count($terms) == 1) {
        $tid = key($terms);
      }
    }
    else {
      // Add term and get the tid.
      $term = new stdClass();
      $term->name = $new['name'];
      $term->vid = $new['vid'];
      $status = taxonomy_term_save($term);
      $tid = $term->tid;
    }
  }

  return $tid;
}

/**
 * Helper function to check if a calendar already exists in {mastercal_calendar}.
 *
 * @return boolean
 *   TRUE if calendar exists.
 *   FALSE if calendar does not exist.
 */
function _mastercal_calendar_exists($cid) {
  return (bool) db_query_range('SELECT 1 FROM {mastercal_calendar} WHERE cid = :cid', 0, 1, array(':cid' => $cid))->fetchField();
}

/**
 * Helper function to check if {mastercal_calendar} is empty.
 *
 * @return boolean
 *   TRUE if any records exist.
 *   FALSE if table is empty.
 */
function _mastercal_records_exist() {
  return (bool) db_query_range('SELECT 1 FROM {mastercal_calendar}', 0, 1)->fetchField();
}

/**
 * Helper function to save a calendar to database. If the $cid does not exist, 
 * a new record is created. Otherwise the fields for the existing calendar are 
 * updated.
 * 
 * @param $cid
 * @param $api_username
 * @param $api_password
 * @param $start_date
 * @param $range
 */
function _mastercal_save($cid, $api_username, $api_password, $start_date, $range) {
  // Check to see if the calendar is in the database.
  if (_mastercal_calendar_exists($cid)) {
    // Update the fields for the existing calendar.
  }
  else {
    // Save the new calendar to the database.
    $return = db_insert('mastercal_calendar')
    ->fields(array(
    'name' => $cid,
    'cid' => (int)$cal['CalendarID'],
    'username' => variable_get('mastercal_default_username'),
    'password' => variable_get('mastercal_default_password'),
    'start_date' => 0,
    'end_date' => 0,
    )
    )
    ->execute();
    return $return;
  }
}

/**
 * Helper function to load calendars from the database.
 *
 * @return object
 *   If no $calendar_id is passed, the default action is to return all calendars
 *   in the database. 
 */
function _mastercal_load($calendar_id = 'c') {
  if ($calendar_id == 'c') {
    // Read all fields from the mastercal_calendar table.
    $query = db_select('mastercal_calendar', 'c');
    $query->fields('c');
    
    // Return the result in object format.
    return $query->execute()->fetchAll();
  }
  else {
    $query = db_select('mastercal_calendar', 'c');
    $query
      ->condition('c.cid', $calendar_id, '=')
      ->fields('c');
    
    return $query->execute()->fetch();
  }
}

/**
 * Helper function to delete a calendar from the database.
 *
 * @param int $cid
 *
 * @return boolean $deleted
 */
function _mastercal_delete($cid) {
  $deleted = db_delete('mastercal_calendar')
  ->condition('cid', $cid)
  ->execute();
  return $deleted;
}
