<?php

$un = base64_decode('QXJ0c19Jb3dhX0FQSQ==');
$pw = base64_decode('QXAzZG45IW5leg==');
//initialize an instance of the mastercalSOAP class then call the GetEvents() method
//the calendars being queried for the events list are loaded from the database
$param = array(
'userName' => $un,
'password' => $pw,
'startDate' => firstOfMonth(),
'endDate' => lastOfMonth(),
'location' => '',
'eventName' => '',
'calendars' => mastercal_load(),
'eventTypes' => Null,
'udqAnswer' => Null,
);

/**
 * @file
 * Administrative page callbacks
 */

function mastercal_uiowa_form($form, &$form_state) {
  $form['description'] = array(
    '#type' => 'item',
    '#title' => t('University of Iowa Module Package'),
    '#markup' => '<p>This package contains various modules developed by ITS
     Web Services.</p>',
  );
  
  return $form;
}

/**
 * Menu page callback: builds the page for administering the site's DEA MasterCalendar integration.
 */
 //create the table of calendars with remove button seen at /admin/config/user-interface/mastercal
function mastercal_admin_form($form, &$form_state) {
  $header = array(t('Name'), array('data' => t('options'), 'colspan' => 4));
  $calendars_in_drupal = mastercal_load();
  
  $calendars = mastercal_getCals($param);
  
  $form['calendars'] = array(
    
  );
  
  $form['table'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#multiple' => FALSE,
    '#empty' => t('No calendars found.'),
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Configure'),
  );
  
  return $form;
}

/**
 * Form callback: builds the form for adding a Calendar.
 *
 * @param $form
 *   An associative array containing the structure of the form.
 * @param $form_state
 *   An associative array containing the current state of the form.
 *
 * @return
 *   An array representing the form definition.
 *
 * @ingroup forms
 * @see mastercal_add_form_validate()
 * @see mastercal_add_form_submit()
 */
function mastercal_add_form($form, &$form_state) {

  $calendars = client()->GetCals();
   //select box where the value of the selection = the calendar id number
   $form['mastercal_name'] = array(
       '#type' => 'select',
       '#title' => t('Calendar'),
       '#options' => $calendars,
       '#default_value' => 0,
       '#description' => t('Master Calendar to add to this site.'),
   );

  $form['actions'] = array('#type' => 'actions');
  //create submit button
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add Calendar'),
  );

  return $form;
}

/**
 * Validation handler for mastercal_add_form().
 */
function mastercal_add_form_validate($form, &$form_state) {
  // Check to prevent a duplicate.
  if (cal_exists($form_state['values']['mastercal_name'])) {
    form_set_error('new', t('The calendar %name already exists. Choose another.', array('%name' => $form_state['values']['mastercal_name'])));
  }
}

/**
 * Submit handler for mastercal_add_form().
 */
function mastercal_add_form_submit($form, &$form_state) {
  $cal = client()->GetCal($form_state['values']['mastercal_name']);
  //print_r($title);
  variable_set('mastercal_auth', True);

  variable_set('mastercal_name', $cal['Name']);
  mastercal_save($cal);
  drupal_set_message(t('The Calendar %cal has been created. You can edit it from this page.', array('%cal' => $cal['Name'])));
  $form_state['redirect'] = 'admin/config/user-interface/mastercal/';
}

/**
 * Form callback: builds the confirmation form for deleting a calendart.
 *
 * @param $form
 *   An associative array containing the structure of the form.
 * @param $form_state
 *   An associative array containing the current state of the form.
 *   
 * @return
 *   An array representing the form definition.
 *
 * @ingroup forms
 * @see shortcut_set_delete_form_submit()
 */
function mastercal_delete_form($form, &$form_state, $cid) {
  $form['cid'] = array(
    '#type' => 'value',
    '#value' => $cid,
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete the Calendar %title?', array('%title' => $cid)),
    'admin/config/user-interface/mastercal/' . $cid,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for mastercal_delete_form().
 */
function mastercal_delete_form_submit($form, &$form_state) {
  $cid = $form_state['values']['cid'];
  mastercal_delete($cid);
  $form_state['redirect'] = 'admin/config/user-interface/mastercal';
  drupal_set_message(t('The Calendar has been removed.'));
  
  // Check whether the {mastercal_source} table is empty. If empty, set mastercal_auth to FALSE.
  if (!records_exist()) {
    variable_set('mastercal_auth', FALSE);
  }
}
