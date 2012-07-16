<?php
/**
 * @file
 * This file includes all the helper functions for the mastercal module.
 */

/**
 * Helper function to return an end date given a start date and a time range.
 * Format is YYYY-MM-DD
 * 
 * @param date $start_date
 * @param int $time_range
 *   In number of months.
 */
function _mastercal_end_date($start_date, $time_range) {
  // Add however many months to the start date.
  $start_date_plus_range = strtotime('+' . $time_range . ' month', $start_date);
  
  // Convert the new date to the correct format.
  $end_date = date('Y-m-d', $start_date_plus_range);
  
  // Return the end date.
  return $end_date;
}

/**
 * Helper function to collapse dates. Changes the EventDate to an array of
 * multiple values. 
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
 * Helper function to save a calendar to database. If the $cid does not exist, 
 * a new record is created. Otherwise the fields for the existing calendar are 
 * updated.
 * 
 * @param $cid
 * @param $username
 * @param $password
 * @param $start_date
 * @param $time_range
 */
function _mastercal_save($calendar_name, $cid, $username, $password, $start_date, $time_range, $connected) {
  // Check to see if the calendar is in the database.
  if (_mastercal_load($cid)) {
    // Update the fields for the existing calendar.
    $return = db_update('mastercal_calendar')
      ->fields(array(
        'name' => $calendar_name,
        'cid' => (int)$cid,
        'username' => $username,
        'password' => $password,
        'start_date' => strtotime($start_date),
        'time_range' => $time_range,
        'connected' => $connected
      ))
      ->condition('cid', $cid, '=')
    ->execute();
    
    return $return;
  }
  else {
    // Save the new calendar to the database.
    $return = db_insert('mastercal_calendar')
      ->fields(array(
        'name' => $calendar_name,
        'cid' => (int)$cid,
        'start_date' => REQUEST_TIME,
      ))
      ->useDefaults(array('username', 'password', 'time_range', 'connected'))
    ->execute();
    
    return $return;
  }
}

/**
 * Helper function to load calendars from the database.
 *
 * @return object
 *   If no $cid is passed, the default action is to return all calendars
 *   in the database. 
 */
function _mastercal_load($cid = 'c') {
  if ($cid == 'c') {
    // Read all fields from the mastercal_calendar table.
    $query = db_select('mastercal_calendar', 'c');
    $query->fields('c');
    
    // Return the result in object format.
    return $query->execute()->fetchAll();
  }
  else {
    $query = db_select('mastercal_calendar', 'c');
    $query
      ->condition('c.cid', $cid, '=')
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

/**
 * Helper function to format a string by replacing certain symbols.
 * 
 * @TODO: Look into encodings. The returned strings from the MC API are 
 * looking funky after running utf8_encode. Removed for now.
 * 
 * @param $string
 */
function _mastercal_format($unformatted_string) {
  // Create array of characters to be replaced.
  $targets = array("\n", '“', '”', "‘", "’", '-', '—', '…');
  
  // Create array of replacments. Must be in order.
  $replacements = array(' ', '&#8220;', '&#8221;', '&#8216;', '&#8217;', '&#8211;', '&#8212;', '&#x2026;');
  
  // Replace the symbols and create a new string.
  $replaced_string = str_replace($targets, $replacements, $unformatted_string); 
 
  // UTF8 encode the replaced string.
  $formatted_string = utf8_encode($replaced_string);
  
  // Return the formatted string.
  return $formatted_string;
}

