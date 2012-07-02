<?php
/**
 * @file
 * This file holds administrative callback functions for the Mastercal module. 
 */

// Include the WSCLient functions.
include_once drupal_get_path('module', 'mastercal') . '/mastercal.wsclient.inc';

/**
 * UIowa Module Package configuration callback.
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
 * Menu page callback: builds the page for administering the site's UIowa Master
 * Calendar integration.
 */
function mastercal_admin_form($form, &$form_state) {
  // Create the header.
  $header = array(
    'title' => t('Title'),
    'username' => t('API Username'),
    'password' => t('API Password'),
    'start_date' => t('Start Date'),
    'end_date' => t('End Date'),
  );
  
  // Get any calendars that have been added already.
  $calendars = _mastercal_load();
  
  // Initialize an empty array.
  $options = array();
  
  // Loop through the individual calendars.
  foreach ($calendars as $calendar) {
    $options[$calendar->cid] = array(
      'title' => $calendar->title,
      'username' => $calendar->username,
      'password' => $calendar->password,
      'start_date' => $calendar->start_date,
      'end_date' => $calendar->end_date,
    );
  }
  
  $form['table'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#multiple' => FALSE,
    '#empty' => t('No calendars found.'),
  );
  
  $form['edit'] = array(
    '#type' => 'submit',
    '#value' => t('Edit'),
  );
  
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );
  
  return $form;
}

/**
 * Admin form validation handler.
 * 
 */
function mastercal_admin_form_validate($form, &$form_state) {
  // Return an error if no calendar is selected from the table.
  // BUG: Required radios with no selection throw an odd error message. 
  // See: http://drupal.org/node/811542
  if ($form_state['values']['table'] == '') {
    form_set_error('table', t('Please select a calendar.'));
  }
  
  // If the 'Edit' submit button is clicked, redirect to the calendar
  // edit form with the table select value passed as an argument.
  if ($form_state['clicked_button']['#value'] == $form_state['values']['edit']) {
    $form_state['redirect'] = 'admin/config/uiowa/mastercal/' . $form_state['values']['table'] . '/edit';
  }
  // If the 'Delete' submit button is clicked, redirect to the delete
  // confirmation page with the table select value passed as an argument.
  else if ($form_state['clicked_button']['#value'] == $form_state['values']['delete']) {
    $form_state['redirect'] = 'admin/config/uiowa/mastercal/' . $form_state['values']['table'] . '/delete';
  }
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
  // Get a list of all calendars available.
  $calendars = _mastercal_get_cals(variable_get('mastercal_default_parameter'));
  
  $form['calendar_id'] = array(
    '#type' => 'select',
    '#title' => t('Calendars'),
    '#options' => $calendars,
    '#required' => TRUE,
    '#description' => t('Select a calendar to add.'),
  );
  
  $form['actions'] = array('#type' => 'actions');
  
  // Create submit button.
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
  if (_mastercal_calendar_exists($form_state['values']['calendar_id'])) {
    form_set_error('new', t('The calendar %name already exists. Choose another.', array('%name' => $form_state['values']['calendar_id'])));
  }
}

/**
 * Submit handler for mastercal_add_form().
 */
function mastercal_add_form_submit($form, &$form_state) {
  // Return the same calendar from the MC API as an array.
  $calendar = _mastercal_get_cal($calendar_id = $form_state['values']['calendar_id']);
 
  // Set persistent variables.
  variable_set('mastercal_auth', TRUE);
  variable_set('mastercal_name', $calendar['Name']);
  
  // Save the calendar in the database.
  _mastercal_save($calendar);
  
  // Set a confirmation message.
  drupal_set_message(t('The Calendar %calendar has been created. You can edit it from this page.', array('%calendar' => $calendar['Name'])));
  
  // Redirect the user back to the calendar overview page.
  $form_state['redirect'] = 'admin/config/uiowa/mastercal/';
}

/**
 * Form callback: builds the form for configuring a calendar.
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
function mastercal_edit_form($form, &$form_state, $cid) {
  $form['cid'] = array(
    '#type' => 'value',
    '#value' => $cid,
  );
  
  $form['api_username'] = array(
    '#type' => 'textfield',
    '#title' => t('API Username'),
    // @TODO: '#default_value' => _mastercal_get_field($cid, 'username'),
    '#required' => TRUE,
  );
  
  $form['api_password'] = array(
    '#type' => 'password_confirm',
    '#title' => t('API Password'),
  );
  
  $form['start_date'] = array(
    '#type' => 'date',
    '#title' => t('Start date'),
    '#description' => t('The date this calendar should start importing events from.'),
    '#required' => TRUE,
  );
  
  $form['end_date'] = array(
    '#type' => 'date',
    '#title' => t('End date'),
    '#description' => t('The date this calendar should stop importing events.'),
    '#required' => TRUE,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  
  return $form;
}

/**
 * Form callback: builds the confirmation form for deleting a calendar.
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
  
  // Check whether the {mastercal_calendar} table is empty. If empty, set 
  // mastercal_auth to FALSE.
  if (!_mastercal_records_exist()) {
    variable_set('mastercal_auth', FALSE);
  }
}
