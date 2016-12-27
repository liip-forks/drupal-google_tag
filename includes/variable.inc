<?php

/**
 * @file
 * Contains the variable definitions.
 *
 * @author Jim Berry ("solotandem", http://drupal.org/user/240748)
 */

/**
 * Implements hook_variable_group_info().
 */
function _google_tag_variable_group_info() {
  $groups['google_tag'] = array(
    'title' => t('Google Tag Manager'),
    'description' => t('Allows your website analytics to be managed using Google Tag Manager.'),
    'access' => 'administer google tag manager',
    'path' => array('admin/config/system/google_tag'),
  );

  return $groups;
}

/**
 * Implements hook_variable_info().
 */
function _google_tag_variable_info($options) {
  // Gather data.
  $groups = ['general', 'path', 'role', 'status', 'advanced'];
  $common = [
    'group' => 'google_tag',
    'localize' => TRUE,
    'multidomain' => TRUE,
  ];

  // Build variables.
  $variables = [];
  foreach ($groups as $group) {
    $function = "_google_tag_variable_info_$group";
    $variables += $function($options);
  }
  foreach ($variables as $key => $variable) {
    $variables[$key] += $common;
  }

  return $variables;
}

/**
 * Implements hook_variable_info().
 */
function _google_tag_variable_info_general($options) {
  // Gather data.
  $args = array('@path' => 'https://tagmanager.google.com/');
  $description = t('The ID assigned by Google Tag Manager (GTM) for this website container. To get a container ID, <a href="@path">sign up for GTM</a> and create a container for your website.', $args);

  // Build variables.
  $variables['google_tag_container_id'] = array(
    'type' => 'string',
    'title' => t('Container ID', array(), $options),
    'description' => $description,
    'default' => '',
    'validate callback' => 'google_tag_validate_container_id',
    'element' => array(
      '#attributes' => array('placeholder' => array('GTM-xxxxxx')),
      '#size' => 12,
      '#maxlength' => 15,
      '#required' => TRUE,
    ),
  );

  return $variables;
}

/**
 * Implements hook_variable_info().
 */
function _google_tag_variable_info_path($options) {
  // Gather data.
  $list_description = t('Enter one relative path per line using the "*" character as a wildcard. Example paths are: "%blog" for the blog page, "%blog-wildcard" for each individual blog, and "%front" for the front page.', array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>'));

  // Build variables.
  $variables['google_tag_path_toggle'] = array(
    'type' => 'select',
    'title' => t('Add snippet on specific paths', array(), $options),
    'options' => array(
      t('All paths except the listed paths', array(), $options),
      t('Only the listed paths', array(), $options),
    ),
    'default' => 0,
  );

  $variables['google_tag_path_list'] = array(
    'type' => 'text',
    'title' => t('Listed paths', array(), $options),
    'description' => $list_description,
    'default' => GOOGLETAGMANAGER_PATHS,
    'validate callback' => '_google_tag_validate_text',
    'element' => array(
      '#rows' => 10,
    ),
  );

  return $variables;
}

/**
 * Implements hook_variable_info().
 */
function _google_tag_variable_info_role($options) {
  // Gather data.
  $roles = array_map('check_plain', user_roles());
  $roles = drupal_map_assoc($roles);

  // Build variables.
  $variables['google_tag_role_toggle'] = array(
    'type' => 'select',
    'title' => t('Add snippet for specific roles', array(), $options),
    'options' => array(
      t('All roles except the selected roles', array(), $options),
      t('Only the selected roles', array(), $options),
    ),
    'default' => 0,
  );

  $variables['google_tag_role_list'] = array(
    'type' => 'options',
    'title' => t('Selected roles', array(), $options),
    'default' => array(),
    'options' => $roles,
  );

  return $variables;
}

/**
 * Implements hook_variable_info().
 */
function _google_tag_variable_info_status($options) {
  // Gather data.
  $args = array('@path' => 'http://en.wikipedia.org/wiki/List_of_HTTP_status_codes');
  $list_description = t('Enter one response status per line. For more information, refer to the <a href="@path">list of HTTP status codes</a>.', $args);

  // Build variables.
  $variables['google_tag_status_toggle'] = array(
    'type' => 'select',
    'title' => t('Add snippet for specific statuses', array(), $options),
    'options' => array(
      t('All statuses except the listed statuses', array(), $options),
      t('Only the listed statuses', array(), $options),
    ),
    'default' => 0,
  );

  $variables['google_tag_status_list'] = array(
    'type' => 'text',
    'title' => t('Listed statuses', array(), $options),
    'description' => $list_description,
    'default' => GOOGLETAGMANAGER_STATUSES,
    'validate callback' => '_google_tag_validate_text',
    'element' => array(
      '#rows' => 5,
    ),
  );

  return $variables;
}

/**
 * Implements hook_variable_info().
 */
function _google_tag_variable_info_advanced($options) {
  // Build variables.
  $variables['google_tag_compact_tag'] = array(
    'type' => 'boolean',
    'title' => t('Compact the JavaScript snippet', array(), $options),
    'description' => t('If checked, then the JavaScript snippet will be compacted to remove unnecessary whitespace. This is <strong>recommended on production sites</strong>. Leave unchecked to output a snippet that can be examined using a JavaScript debugger in the browser.', array(), $options),
    'default' => 1,
  );

  $variables['google_tag_data_layer'] = array(
    'type' => 'string',
    'title' => t('Data layer', array(), $options),
    'description' => t('The name of the data layer. Default value is "dataLayer". In most cases, use the default.'),
    'default' => 'dataLayer',
    'validate callback' => '_google_tag_validate_text',
    'element' => array(
      '#attributes' => array('placeholder' => array('dataLayer')),
      '#required' => TRUE,
    ),
  );

  return $variables;
}

/**
 * Element validation handler for google_tag_container_id.
 *
 * @todo This may suffice for all the elements as only the container_id has an
 *   error message. Error messages could be done for other elements by calling
 *   form_set_error('element[name]', $message).
 */
function _google_tag_validate_container_id($variable, $options, $element, $form, &$form_state) {
  // Use this routine as the variable realm form has nested values.
  $value = drupal_array_get_nested_value($form_state['values'], $element['#parents']);
  // Trim the text value.
  $value = trim($value);
  // Replace all types of dashes (n-dash, m-dash, minus) with a normal dash.
  $value = str_replace(array('–', '—', '−'), '-', $value);
  // Replace the text value.
  drupal_array_set_nested_value($form_state['values'], $element['#parents'], $value);

  if (!preg_match('/^GTM-\w{4,}$/', $value)) {
    // @todo Is there a more specific regular expression that applies?
    // @todo Is there a way to "test the connection" to determine a valid ID for
    // a container? It may be valid but not the correct one for the website.
    return t('A valid container ID is case sensitive and formatted like GTM-xxxxxx.');
  }
}

/**
 * Element validation handler for text values.
 */
function _google_tag_validate_text($variable, $options, $element, $form, &$form_state) {
  $value = drupal_array_get_nested_value($form_state['values'], $element['#parents']);
  google_tag_text_clean($value);
  drupal_array_set_nested_value($form_state['values'], $element['#parents'], $value);
}