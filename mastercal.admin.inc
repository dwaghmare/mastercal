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
    'connected' => t('Connected'),
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
      'connected' => ($calendar->connected == '1') ? 'Yes' : 'No',
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
  elseif ($form_state['clicked_button']['#value'] == $form_state['values']['delete']) {
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
    form_set_error('cid', t('The calendar %name already exists. Choose another.', array('%name' => $form_state['values']['cid'])));
  }
}

/**
 * Submit handler for mastercal_add_form().
 */
function mastercal_add_form_submit($form, &$form_state) {
  // Return the same calendar from the MC API as an array.
  $calendar = _mastercal_get_cal($form_state['values']['cid']);
  
  // Save the calendar in the database.
  _mastercal_save($calendar['Name'], $form_state['values']['cid'], NULL, NULL, NULL, NULL, NULL, NULL);
  
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
  
  // Store the connected value. Assumes a calendar has not been connected.
  $form['connected'] = array(
    '#type' => 'value',
    '#value' => $calendar->connected,
  );
  
  // API username.
  $form['username'] = array(
    '#type' => 'textfield',
    '#title' => t('API Username'),
    '#default_value' => $calendar->username,
    '#required' => TRUE,
  );
  
  // API password.
  $form['password'] = array(
    '#type' => 'password_confirm',
    '#title' => t('API Password'),
  );
  
  // Start date.
  $form['start_date'] = array(
    '#type' => 'textfield',
    '#title' => t('Start date'),
    '#description' => t('The date this calendar should start importing events from.'),
    '#required' => TRUE,
    '#attributes' => array('class' => array('pickadate')),
    '#default_value' => format_date($calendar->start_date, $type='custom', $format='m/d/Y'),
  );
  
  // Time range.
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
  
  // If Workbench Access is enabled.
  if (module_exists('workbench_access')) {
    // Workbench fieldset
    $form['workbench'] = array(
    '#type' => 'fieldset',
    '#title' => t('Workbench'),
    );
    
    if (variable_get('workbench_access_allow_multiple') == 1) {
      // Workbench Access with multiple editorial sections per node.
      $form['workbench']['access'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Workbench Access'),
      '#description' => t('Select the Workbench Access editorial sections that should be assigned to imported events. Users and roles with access to the selected editorial sections will have access to create/edit events.'),
      '#options' => _mastercal_get_workbench_access_editorial_sections(),
      );
    }
    else {
      // Workbench Access with one editorial section per node.
      $form['workbench']['access'] = array(
        '#type' => 'select',
        '#title' => t('Workbench Access'),
        '#description' => t('Select the Workbench Access editorial section that should be assigned to imported events. Users and roles with access to the selected editorial section will have access to create/edit events.'),
        '#options' => _mastercal_get_workbench_access_editorial_sections(),
      );
    }
  }
  
  // If the calendar is connected.
  if ($calendar->connected == 1) {
    // Get the UDQs for the calendar.
    $udqs = _mastercal_get_udqs($cid, NULL);
    
    // Create another array for the taxonomy terms.
    $terms = array();
    
    // Assign each udq as key => value pair.
    foreach ($udqs as $udq) {
      $terms[$udq['UDQID']] = t($udq['PromptText']);
    }
    
    // Taxonomy fieldset.
    $form['taxonomy'] = array(
      '#type' => 'fieldset',
      '#title' => t('Taxonomy'),
    );
    
    // If no terms have been set, the default value will not work.
    if ($calendar->terms != NULL) {
      // Create the taxonomy term checkboxes.
      $form['taxonomy']['terms'] = array(
      '#type' => 'checkboxes',
      '#options' => $terms,
      '#description' => t('Select the fields that should be used as taxonomy terms in field_mc_tags.'),
      '#default_value' => unserialize($calendar->terms),
      );
    }
    else {
      // Create the taxonomy term checkboxes.
      $form['taxonomy']['terms'] = array(
      '#type' => 'checkboxes',
      '#options' => $terms,
      '#description' => t('Select the fields that should be used as taxonomy terms in field_mc_tags.'),
      );
    }
    
    // Markup.
    $form['udq_markup'] = array(
      '#markup' => t('<h3>UDQs</h3><p>For each UDQ below, select the data type 
      that Drupal should use for the corresponding field. Incorrect data types 
      will be truncated. Leave the data type set to "None" if you do not
      wish to create a field for a given UDQ.</p>'),
    );
    
    // Iterate over each UDQ.
    foreach ($udqs as $udq) {
      // Create a concatenated string for this fieldset.
      $udq_fieldset = 'udq_' . $udq['UDQID'] . '_fieldset';
      
      // Assign the UDQID to a variable.
      $udqid = 'udq_' . $udq['UDQID'];
      
      // UDQ fieldset.
      $form[$udq_fieldset] = array(
        '#type' => 'fieldset',
        '#title' => t('UDQ %udq Settings', array('%udq' => $udq['UDQID'])),
      );
    
      // UDQ prompt text field.
      $form[$udq_fieldset][$udqid . '_prompt_text'] = array(
        '#type' => 'textfield',
        '#title' => t('Prompt Text'),
        '#default_value' => $udq['PromptText'],
        '#disabled' => TRUE,
      );
    
      // UDQ Sample Answer.
      $form[$udq_fieldset][$udqid . '_sample_answer'] = array(
        '#type' => 'textfield',
        '#title' => t('Sample Answer'),
        '#default_value' => (!empty($udq['Answer']) ? $udq['Answer'] : ''),
        '#disabled' => TRUE,
      );
    
      // UDQ Data Type.
      $form[$udq_fieldset][$udqid . '_data_type'] = array(
        '#type' => 'select',
        '#title' => t('Data Type'),
        '#description' => t('The data type for the field.'),
        '#options' => array(
          'text' => t('Text'),
          'number_float' => t('Number'),
          'datetime' => t('Date'),
        ),
        '#empty_value' => t('-None-'),
        '#default_value' => _mastercal_get_udq_field_type('field_mc_calendar_' . $calendar->cid . '_' . $udqid),
      );
    }  
  }
  
  // Submit button.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  
  // Return the form.
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
  // Assign the form elements to variables.
  $cid = $form_state['values']['cid'];
  $username = $form_state['values']['username'];
  $password = $form_state['values']['password'];
  $start_date = date('Y-m-d', strtotime($form_state['values']['start_date']));
  $time_range = $form_state['values']['time_range'];
  $connected = $form_state['values']['connected'];
  
  // Load the calendar from the database as an object.
  $calendar = _mastercal_load($cid);
  
  // If the API password has not been changed, load the password from the 
  // database.
  if (empty($password)) {
    $password = $calendar->password;
  }
  
  // Attempt to connect to the API.
  // This has to be done without the helper function to test the new values.
    
  // Create the end date.
  $end_date = _mastercal_end_date(strtotime($start_date), $time_range);
    
  // Construct the calendar list in the format required by the API.
  $calendar_list = array();
  $calendar_list['int'][] = $cid;
    
  // Construct the API paramter as an array for the GetEvents call.
  $param = array(
    'userName' => $username,
    'password' => $password,
    'startDate' => $start_date,
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
    
  // If the result contains a login error, return a form error.
  // @TODO: There is probably a better way to check for this.
  if (strpos($result->GetEventsResult, 'Login Failed.')) {
    form_set_error('username', 'Cannot connect with the given username and password. Please check your spelling or contact the UIowa Master Calendar team.'); 
  }
  else {
    // The credentials worked. Set the connected value to 1 (true).
    $form_state['values']['connected'] = 1;
  }
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
  $connected = $form_state['values']['connected'];
  $terms = (!empty($form_state['values']['terms'])) ? serialize($form_state['values']['terms']) :  NULL;
  
  // Load the calendar from the database as an object.
  $calendar = _mastercal_load($cid);
  
  // If the API password has not been changed, load the password from the 
  // database and just re-save it again.
  if (empty($password)) {
    $password = $calendar->password;
  }
  
  // If the calendar is connected.
  if ($calendar->connected) {
    // Get the UDQs.
    $udqs = _mastercal_get_udqs($cid);
    
    // Iterate over each UDQ.
    foreach ($udqs as $udq) {
      // Create a reusable variable for form_state values.
      $udq_prefix = 'udq_' . $udq['UDQID'] . '_';
    
      // Check to see if this UDQ has been mapped.
      if ($form_state['values'][$udq_prefix . 'data_type'] != '-None-') {
        // Check to see if the field already exists.
        if (!field_info_field('field_mc_calendar_' . $calendar->cid . '_udq_' . $udq['UDQID'])) {
          // Create and attach the field.
          _mastercal_create_field($form_state['values'][$udq_prefix . 'data_type'], 'field_mc_calendar_' . $calendar->cid . '_udq_' . $udq['UDQID'], $form_state['values'][$udq_prefix .'prompt_text']);
        }
      }
    }
  }
  
  // Save the calendar.
  _mastercal_save($calendar->name, $cid, $username, $password, $start_date, $time_range, $connected, $terms);
  
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
