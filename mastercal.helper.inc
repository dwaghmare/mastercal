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
 * Helper function to format a date returned from the MC API. Necessary to save
 * the date to a field.
 * 
 * @param $date
 */
function _mastercal_format_date($date) {
  // Explode into an array on 'T' character.
  $date_array = explode('T', $date);
  
  // Explode date_array into array on '-' character.
  $time_array = explode('-', $date_array[1]);
  
  // Concatenate date and time into datetime format.
  $datetime = $date_array[0] . ' ' . $time_array[0];
  
  // Format the date using gmdate().
  $formatted_date = gmdate('Y-m-d H:i:s', strtotime($datetime));
  
  return $formatted_date;
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
 * Helper function to create taxonomy term.
 *
 * @param $term
 */
function _mastercal_create_term($term) {

  // Grabs the list of vocabularies.
  $vocabularies = taxonomy_get_vocabularies();
  
  // Get the vid of the Master Calendar event tags vocabulary.
  foreach ($vocabularies as $voc) {
    if ($voc->machine_name == 'mastercal_event_tags') {
      $vid = $voc->vid;
    }
  }
  
  // Grabs the terms from the chosen vocabulary.
  if (!empty($vid)) {
    $new = array('vid' => $vid, 'name' => $term);
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
function _mastercal_save($calendar_name, $cid, $username, $password, $start_date, $time_range, $connected, $terms) {
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
        'connected' => $connected,
        'terms' => $terms,
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
function _mastercal_format_string($unformatted_string) {
  // Strip the inline styles from the html elements.
  $stripped_string = preg_replace("/<([a-z][a-z0-9]*)[^>]*?(\/?)>/i", '<$1$2>', $unformatted_string);
  
  // Create array of characters to be replaced.
  $targets = array('<!-- ?xml:namespace prefix = o ns = "urn:schemas-microsoft-com:office:office" ? -->', "\n", '“', '”', "‘", "’", '-', '—', '…');
  
  // Create array of replacments. Must be in order.
  $replacements = array(' ', ' ', '&#8220;', '&#8221;', '&#8216;', '&#8217;', '&#8211;', '&#8212;', '&#x2026;');
  
  // Replace the symbols and create a new string.
  $replaced_string = str_replace($targets, $replacements, $stripped_string); 

  // UTF8 encode the replaced string.
  $formatted_string = utf8_encode($replaced_string);
  
  // Return the formatted string.
  return $formatted_string;
}

/**
 * Helper function to create field and attach it to mastercal_event node.
 * 
 * @param $field_type
 * @param $field_name
 * @param $field_label
 */
function _mastercal_create_field($field_type, $field_name, $field_label) {
  // Create a field array.
  $field = array(
    'type' => $field_type,
    'field_name' => $field_name,
    'active' => '1',
    'cardinality' => '1',
    'deleted' => '0',
    
  );
  
  // Create the field.
  field_create_field($field);
  
  // Create the array for the text instance of this field.
  $instance = array(
    'field_name' => $field_name,
    'entity_type' => 'node',
    'label' => $field_label,
    'bundle' => 'mastercal_event',
    'deleted' => '0',
    'description' => 'UDQ field settings.'
  );
  
  // Set widget data depending on the data type.
  if ($field_type == 'text') {
   $instance['widget'] = array(
     'type' => 'textfield',
   );
  }
  else if ($field_type == 'number_float') {
    $instance['widget'] = array(
      'type' => 'textfield',
    );
  }
  else if ($field_type == 'datetime') {
    $field['module'] = 'date';
    
    $field['settings'] = array(
      'granularity' => array(
        'day' => 'day',
        'hour' => 'hour',
        'minute' => 'minute',
        'month' => 'month',
        'second' => 0,
        'year' => 'year',
      ),
      'repeat' => 0,
      'timezone_db' => 'UTC',
      'todate' => 'never',
      'tz_handling' => 'site',
    );
    
    $instance['widget'] = array(
      'active' => 1,
      'module' => 'date',
      'settings' => array(
        'increment' => '15',
        'input_format' => 'm/d/Y - H:i:s',
        'input_format_custom' => '',
        'label_position' => 'above',
        'repeat_collapsed' => 0,
        'text_parts' => array(),
        'year_range' => '-3:+3',
      ),
      'type' => 'date_popup',
    );
  }
  
  // Attach the field to the mastercal_event node.
  field_create_instance($instance);
}

/**
 * Helper function to return the field type given a field name. Used in edit 
 * form default value for UDQ select list.
 * 
 * @param $field_name
 */
function _mastercal_get_udq_field_type($field_name) {
  // Check to see if the field exists.
  if (field_info_field($field_name)) {
    // Assign the field object.
    $field = field_info_field($field_name);
    
    // Get the field type.
    $field_type = $field['type'];
    
    // Return the field type.
    return $field_type;
  }
  else {
    // Field is not mapped.
    return NULL;
  }
}

