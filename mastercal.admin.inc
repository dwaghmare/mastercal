<?php
/**
 * @file
 * This file holds administrative callback functions for the Mastercal module. 
 */

// Include the helper functions.
include_once drupal_get_path('module', 'mastercal') . '/mastercal.helper.inc';

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
  // @TODO: Sort this by calendar name.
  $header = array(
    'name' => t('Name'),
    'username' => t('API Username'),
    'password' => t('API Password'),
    'start_date' => t('Start Date'),
    'time_range' => t('Range'),
  );
  
  // Get any calendars that have been added already.
  $calendars = _mastercal_load();
  
  // Initialize an empty array.
  $options = array();
  
  // Loop through the individual calendars.
  foreach ($calendars as $calendar) {
    $options[$calendar->cid] = array(
      'name' => $calendar->name,
      'username' => $calendar->username,
      'password' => (!empty($calendar->password)) ? t('************') : '',
      'start_date' => format_date($calendar->start_date, $type='custom', $format='F d, Y'),
      'time_range' => $calendar->time_range . ' month(s)',
    );
  }
  
  // Construct the table select which stores the cid in the selected value.
  $form['table'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#multiple' => FALSE,
    '#empty' => t('No calendars found.'),
  );
  
  // Edit button.
  $form['edit'] = array(
    '#type' => 'submit',
    '#value' => t('Edit'),
  );
  
  // Delete button.
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
 * Form callback: builds the form for adding a calendar.
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
  $calendars = _mastercal_get_cals();
  
  $form['cid'] = array(
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
  if (_mastercal_load($form_state['values']['cid'])) {
    form_set_error('new', t('The calendar %name already exists. Choose another.', array('%name' => $form_state['values']['cid'])));
  }
}

/**
 * Submit handler for mastercal_add_form().
 */
function mastercal_add_form_submit($form, &$form_state) {
  // Return the same calendar from the MC API as an array.
  $calendar = _mastercal_get_cal($form_state['values']['cid']);
  
  // Save the calendar in the database.
  _mastercal_save($calendar['Name'], $form_state['values']['cid'], NULL, NULL, NULL, NULL);
  
  // Set a confirmation message.
  drupal_set_message(t('The calendar %calendar has been created. You can edit it from this page.', array('%calendar' => $calendar['Name'])));
  
  // Redirect the user back to the calendar overview page.
  $form_state['redirect'] = 'admin/config/uiowa/mastercal/';
}

/**
 * Form callback: builds the form for editing a calendar.
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
  // Require jQuery dependencies for building a datepicker.
  drupal_add_library('system', 'ui.datepicker');
  
  // Add inline JavaScript to change text fields to datepickers.
  drupal_add_js('jQuery(document).ready(function(){jQuery(".pickadate").datepicker({
    dateFormat: "mm/dd/yy",
    autoSize: true
  });});', 'inline');
  
  // Get the calendar from the database as an object.
  $calendar = _mastercal_load($cid);
  
  // Display the name of the calendar.
  $form['title'] = array(
    '#markup' => t('<h2>%calendar_name</h2>', array('%calendar_name' => $calendar->name)),
  );
  
  // Store the cid.
  $form['cid'] = array(
    '#type' => 'value',
    '#value' => $cid,
  );
  
  // API username
  $form['username'] = array(
    '#type' => 'textfield',
    '#title' => t('API Username'),
    '#default_value' => $calendar->username,
    '#required' => TRUE,
  );
  
  // API password
  $form['password'] = array(
    '#type' => 'password_confirm',
    '#title' => t('API Password'),
  );
  
  // Start date
  $form['start_date'] = array(
    '#type' => 'textfield',
    '#title' => t('Start date'),
    '#description' => t('The date this calendar should start importing events from.'),
    '#required' => TRUE,
    '#attributes' => array('class' => array('pickadate')),
    '#default_value' => format_date($calendar->start_date, $type='custom', $format='m/d/Y'),
  );
  
  // Time range
  $form['time_range'] = array(
    '#type' => 'select',
    '#title' => t('Range'),
    '#description' => t("How far into the future events should be imported. 
    Calculated daily, i.e. today's date plus 3 months, tomorrow's date plus 3 months, etc."),
    '#required' => TRUE,
    '#options' => array(
      '1' => '1 month',
      '2' => '2 months',
      '3' => '3 months',
      '6' => '6 months',
      '12' => '1 year',
    ),
    '#default_value' => $calendar->time_range,
  );
  
  // Submit button
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  
  return $form;
}

/**
 * Edit form validation handler. Verifies that the API username and password
 * can connect to the Master Calendar server. If not, returns an error.
 * 
 * @param $form
 * @param $form_state
 */
function mastercal_edit_form_validate($form, &$form_state) {
 // @TODO: Implment validation.
}

/**
 * Submit handler for edit form.
 * 
 * @param $form
 * @param $form_state
 */
function mastercal_edit_form_submit($form, &$form_state) {
  // Assign the form elements to variables.
  $cid = $form_state['values']['cid'];
  $username = $form_state['values']['username'];
  $password = $form_state['values']['password'];
  $start_date = $form_state['values']['start_date'];
  $time_range = $form_state['values']['time_range'];
  
  // Load the calendar from the database as an object.
  $calendar = _mastercal_load($cid);
  
  // If the API password has not been changed, load the password from the 
  // database and just re-save it again.
  if (empty($password)) {
    $password = $calendar->password;
  }
  
  // Save the calendar.
  _mastercal_save($calendar->name, $cid, $username, $password, $start_date, $time_range);
  
  // Set a confirmation message.
  drupal_set_message(t('The %calendar_name calendar has been saved.', array('%calendar_name' => $calendar->name)));
  
  // Redirect to mastercal admin form.
  $form_state['redirect'] = 'admin/config/uiowa/mastercal';
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
  // Load the calendar as an object.
  $calendar = _mastercal_load($cid);
  
  $form['cid'] = array(
    '#type' => 'value',
    '#value' => $cid,
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete the calendar %calendar_name?', array('%calendar_name' => $calendar->name)),
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
  _mastercal_delete($cid);
  $form_state['redirect'] = 'admin/config/uiowa/mastercal';
  drupal_set_message(t('The calendar has been removed.'));
}
