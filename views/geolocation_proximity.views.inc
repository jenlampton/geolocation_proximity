<?php
/**
 * @file
 * Contains Views hooks.
 */

/**
 * Implements hook_field_views_data_alter()
 */
function geolocation_proximity_field_views_data_alter(&$result, $field, $module) {
  if ($module == 'geolocation') {
    $field_name = $field['field_name'];
    foreach ($result as &$r) {
      $r[$field_name . '_distance'] = array(
        'group' => 'Geolocation',
        'title' => t('Distance'),
        'help' => t('Filter view results by distance, using geolocation data from %field.', array('%field' => $field_name)),
        'title short' => t('@field: distance', array('@field' => $field_name)),
        'filter' => array(
          'handler' => 'geolocation_proximity_views_handler_filter_distance',
        ),
        'field' => array(
          'handler' => 'geolocation_proximity_views_handler_field_distance',
          // This is use by the table display plugin.
          'click sortable' => TRUE,
        ),
      );
    }
  }
}

/**
 * Implements hook_views_data_alter()
 */
function geolocation_proximity_views_data_alter(&$data) {

  return;

  $data = array();
  foreach (field_info_fields() as $field) {
    if ($field['type'] != 'geolocation_latlng') {
      continue;
    }

    $field_name = 'field_data_' . $field['field_name'];
    $data[$field_name][$field['field_name']]['filter'] = array(
      'handler' => 'geolocation_proximity_views_handler_filter_distance',
    );
  }
  return $data;
}

/*
 * Returns SQL fragment to calculate distance between filter and field.
 *
 * @param float $filter_lat
 *   Filter latitude value (degrees).
 * @param float $filter_lng
 *   Filter longitude value (degrees).
 * @param float $field_latsin
 *   Pre-calculated field latitude sine (radians) using sin(deg2rad(lat)).
 * @param float $field_latcos
 *   Pre-calculated field latitude cosine (radians) using cos(deg2rad(lat)).
 * @param float $field_lng
 *   Pre-calculated field longitude (radians).
 *
 * @return string
 *   SQL fragment to use in views query.
 *   This is the same query that Google suggests.
 *   @see https://developers.google.com/maps/articles/geospatial
 */
function _proximity_sql_fragment($filter_lat, $filter_lng, $field_latsin, $field_latcos, $field_lng) {
  // Pre-calculate filter values.
  $filter_latcos = cos(deg2rad($filter_lat));
  $filter_latsin = sin(deg2rad($filter_lat));
  $filter_lng = deg2rad($filter_lng);

  // Keep it simple. We don't need high accuracy here.
  $earth_radius = 6371;

  return "(
    ACOS(
      $filter_latcos
      * $field_latcos
      * COS( $filter_lng - $field_lng  )
      +
      $filter_latsin
      * $field_latsin
    ) * $earth_radius
  )";
}
