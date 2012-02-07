<?php


/**
 * @file
 * Administrative page callbacks
 */





/**
 * Menu page callback: builds the page for administering the site's DEA MasterCalendar integration.
 */
 //create the table of calendars with remove button seen at /admin/config/user-interface/mastercal
function mastercal_set_admin() {
  $saved_cals = mastercal_load();
  $header = array(t('Name'), array('data' => t('options'), 'colspan' => 4));

  $rows = array();
  foreach ($saved_cals as $cal) {
    $row = array(
      check_plain($cal->title),
      '',
      '',
    );
 
    $row[] = l(t('remove'), "admin/config/user-interface/mastercal/$cal->cid/delete");   
    $rows[] = $row;
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
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
 * @see mastercal_set_add_form_validate()
 * @see mastercal_set_add_form_submit()
 */
function mastercal_set_add_form($form, &$form_state) {

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
 * Validation handler for mastercal_set_add_form().
 */
function mastercal_set_add_form_validate($form, &$form_state) {
  // Check to prevent a duplicate.
  if (cal_exists($form_state['values']['mastercal_name'])) {
    form_set_error('new', t('The calendar %name already exists. Choose another.', array('%name' => $form_state['values']['mastercal_name'])));
  }
}

/**
 * Submit handler for mastercal_set_add_form().
 */
function mastercal_set_add_form_submit($form, &$form_state) {
  $cal = client()->GetCal($form_state['values']['mastercal_name']);
  //print_r($title);
  variable_set('MASTERCAL_AUTH', True);

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
function mastercal_set_delete_form($form, &$form_state, $cid) {
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
 * Submit handler for mastercal_set_delete_form().
 */
function mastercal_set_delete_form_submit($form, &$form_state) {
  $cid = $form_state['values']['cid'];
  mastercal_delete($cid);
  $form_state['redirect'] = 'admin/config/user-interface/mastercal';
  drupal_set_message(t('The Calendar has been removed.'));
  
  // Check whether the {mastercal_source} table is empty. If empty, set MASTERCAL_AUTH to FALSE.
  if (!records_exist()) {
    variable_set('MASTERCAL_AUTH', FALSE);
  }
}


