<?php
/**
 * @file
 * This file includes all the helper functions for the mastercal module.
 */

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

  //grabs the list of vocabularies
  $vocabularies = taxonomy_get_vocabularies();
  foreach ($vocabularies as $voc) {
    if ($voc->machine_name == 'event_types') {
      $vid = $voc->vid;
    }
  }
  //grabs the terms from the chosen vocabulary
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

  //grabs the list of vocabularies
  $vocabularies = taxonomy_get_vocabularies();
  foreach ($vocabularies as $voc) {
    if ($voc->machine_name == 'event_types') {
      $vid = $voc->vid;
    }
  }
  //grabs the terms from the chosen vocabulary
  if (!empty($vid)) {
    $new = array('vid' => $vid, 'name' => $event_type);
    $terms = taxonomy_get_term_by_name($new['name']);
    if (!empty($terms)) {
      if (count($terms) == 1) {
        $tid = key($terms);
      }
    }
    else {
      // add term and get the tid
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
 * Helper function to check if a calendar already exists in {mastercal_source}.
 *
 * @return boolean
 *   TRUE if calendar exists.
 *   FALSE if calendar does not exist.
 */
function _mastercal_calendar_exists($cid) {
  return (bool) db_query_range('SELECT 1 FROM {mastercal_source} WHERE cid = :cid', 0, 1, array(':cid' => $cid))->fetchField();
}

/**
 * Helper function to check if {mastercal_source} is empty.
 *
 * @return boolean
 *   TRUE if any records exist.
 *   FALSE if table is empty.
 */
function _mastercal_records_exist() {
  return (bool) db_query_range('SELECT 1 FROM {mastercal_source}', 0, 1)->fetchField();
}

function _mastercal_save(&$cal) {
  // Save the calendar ID and its name to teh database
  $return = db_insert('mastercal_source')
  ->fields(array(
    'title' => $cal['Name'],
    'cid' => (int)$cal['CalendarID'],
  )
  )
  ->execute();
  return $return;
}

/**
 * Helper function to load calendars from database.
 *
 * @return array $cal_list
 */
function _mastercal_load() {
  //for simple queries it is more economical to use db_query and SQL than to use the new Drupal 7 methods
  $result = db_query("SELECT title, cid FROM {mastercal_source}");

  $cal_ids = $result;
  $cal_list = array();
  //make sure the lsit isnt empty
  if (isset($cal_ids)) {
    //for the soap handler to identify your array as an integer array you have to make an associative array where 'int' => array(12,32,43)
    foreach ($cal_ids as $item) {
      $cal_list['int'][] = $item->cid;
    }
    return $cal_list;
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
  $deleted = db_delete('mastercal_source')
  ->condition('cid', $cid)
  ->execute();
  return $deleted;
}