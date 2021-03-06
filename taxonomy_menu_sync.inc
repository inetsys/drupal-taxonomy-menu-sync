<?php

/**
 * @param string $menu_name
 * @return object|boolean Object taxonomy vocabulary or false if menu is not synchronized
 */
function taxonomy_menu_sync_get_synchronized_vocabulary($menu_name){
	$config = variable_get('taxonomy_menu_sync_list', array());

	return isset($config[$menu_name]) && $config[$menu_name] ? taxonomy_vocabulary_machine_name_load($config[$menu_name]['vocabulary']) : false;
}

/**
 * @param int $tid
 * @param int $mlid
 * @return boolean
 */
function taxonomy_menu_sync_add_relation($tid, $mlid){
	db_insert('taxonomy_menu_sync_relations')
		->fields(array(
			'tid' => $tid,
			'mlid' => $mlid,
		))
		->execute();

	return true;
}

/**
 * @param int $tid
 * @return boolean
 */
function taxonomy_menu_sync_delete_relation($tid, $mlid){
	db_delete('taxonomy_menu_sync_relations')
		->condition('tid', $tid)
		->condition('mlid', $mlid)
		->execute();

	return true;
}

/**
 * @param int $tid
 * @return boolean
 */
function taxonomy_menu_sync_delete_all_relations($tid){
	db_delete('taxonomy_menu_sync_relations')
		->condition('tid', $tid)
		->execute();

	return true;
}

/**
 * @param int $tid
 * @return array
 */
function taxonomy_menu_get_term_relations($tid){
	return db_select('taxonomy_menu_sync_relations', 'tr')
		->fields('tr', array('mlid'))
		->condition('tr.tid', $tid)
		->execute()
		->fetchCol();
}

/**
 * @param int $mlid
 * @return object|boolean Object taxonomy term or false if it does not exist
 */
function taxonomy_term_sync_get_taxonomy_term($mlid){
	$tid = 0;
	if($mlid){
		$tid = db_select('taxonomy_menu_sync_relations', 'tm_rel')
			->fields('tm_rel', array('tid'))
			->condition('tm_rel.mlid', $mlid)
			->execute()
			->fetchField();
	}

	return $tid ? taxonomy_term_load($tid) : false;
}

/**
 * @param int $vid
 * @param string $name
 * @param int $parent_tid
 * @return object|boolean Object taxonomy term or false if it does not exist
 */
function taxonomy_menu_sync_search_existing_term($vid, $name, $parent_tid, $exclude = null){
	$query = db_select('taxonomy_term_data', 'td');
	$query->join('taxonomy_term_hierarchy', 'th', 'td.tid = th.tid');
	$query
		->fields('td' ,array('tid'))
		->condition('td.vid', $vid)
		->condition('td.name', $name)
		->condition('th.parent', $parent_tid);

	if($exclude){
		$query->condition('td.tid', $exclude, '!=');
	}

	$tid = $query
		->execute()
		->fetchField();

	return $tid ? taxonomy_term_load($tid) : false;
}

/**
 * @param int $tid
 * @return object|boolean Object taxonomy term or false if it does not exist
 */
function taxonomy_menu_sync_get_parent_term($tid){
	$parents = taxonomy_get_parents($tid);

	return $parents ? reset($parents) : false;
}

/**
 * @param int $tid
 * @return array List of object taxonomy term
 */
function taxonomy_menu_sync_get_children_terms($tid){
	$query = db_select('taxonomy_term_data', 'td');
	$query->join('taxonomy_term_hierarchy', 'th', 'td.tid = th.tid');
	$tids = $query
		->fields('td' ,array('tid'))
		->condition('th.parent', $tid)
		->orderBy('td.weight', 'asc')
		->execute()
		->fetchCol();

	return $tids ? taxonomy_term_load_multiple($tids) : array();
}

/**
 * @param object $term_from
 * @param object $term_to
 */
function taxonomy_menu_sync_move_term_children($term_from, $term_to){
	if($children = taxonomy_menu_sync_get_children_terms($term_from->tid)){
		foreach($children as $child){
			_taxonomy_menu_sync_move_child_term($child, $term_to);
		}
	}
}
function _taxonomy_menu_sync_move_child_term($term, $parent){
	$vid = $parent ? $parent->vid : $term->vid;
	$parent_tid = $parent ? $parent->tid : 0;

	if($existing = taxonomy_menu_sync_search_existing_term($vid, $term->name, $parent_tid, $term->tid)){
		// Move children to existing term
		if($children = taxonomy_menu_sync_get_children_terms($term->tid)){
			foreach($children as $child){
				_taxonomy_menu_sync_move_child_term($child, $existing);
			}
		}

		$relations = taxonomy_menu_get_term_relations($term->tid);
		if(count($relations) == 1){
			// Let other modules modify anything about this new term
			module_invoke_all('taxonomy_term_sync_modified', $existing, $term);
			// Delete old term
			taxonomy_term_delete($term->tid);
		}
	}else{
		$relations = taxonomy_menu_get_term_relations($term->tid);
		if(count($relations) == 1){
			$term->parent = $parent_tid;
			taxonomy_term_save($term);
		}else{
			$new_term = new stdClass();
			$new_term->vid = $vid;
			$new_term->vocabulary_machine_name = $term->vocabulary_machine_name;
			$new_term->name = $term->name;
			$new_term->weight = $term->weight;
			$new_term->parent = $parent_tid;

			taxonomy_term_save($new_term);
		}
	}
}

/**
 * @param array $item
 * @param object $term
 */
function taxonomy_menu_sync_update_children_items_terms($item, $term){
	if($children = taxonomy_term_sync_get_menu_item_children($item)){
		foreach($children as $mlid => $child){
			_taxonomy_menu_sync_update_child_item($child, $term);
		}
	}
}

/**
 * @param array $item
 * @param object $parent
 */
function _taxonomy_menu_sync_update_child_item($item, $parent){
	$term = taxonomy_term_sync_get_taxonomy_term($item['mlid']);

	$vid = $parent ? $parent->vid : $term->vid;
	$parent_tid = $parent ? $parent->tid : 0;

	if($existing = taxonomy_menu_sync_search_existing_term($vid, $term->name, $parent_tid, $term->tid)){
		// Move children referenced terms to existing term
		if($children = taxonomy_term_sync_get_menu_item_children($item)){
			foreach($children as $child){
				_taxonomy_menu_sync_update_child_item($child, $existing);
			}
		}

		$relations = taxonomy_menu_get_term_relations($term->tid);
		if(count($relations) == 1){
			// Let other modules modify anything about this new term
			module_invoke_all('taxonomy_term_sync_modified', $existing, $term);
			// Delete old term
			taxonomy_term_delete($term->tid);
		}
		// Delete old reference
		taxonomy_menu_sync_delete_relation($term->tid, $item['mlid']);
		// Set new reference
		taxonomy_menu_sync_add_relation($existing->tid, $item['mlid']);

	}else{
		$relations = taxonomy_menu_get_term_relations($term->tid);
		if(count($relations) == 1){
			$term->parent = $parent_tid;
			taxonomy_term_save($term);

			// Continue with item children
			if($children = taxonomy_term_sync_get_menu_item_children($item)){
				foreach($children as $child){
					_taxonomy_menu_sync_update_child_item($child, $term);
				}
			}
		}else{
			// Create new term
			$new_term = new stdClass();
			$new_term->vid = $vid;
			$new_term->vocabulary_machine_name = $term->vocabulary_machine_name;
			$new_term->name = $term->name;
			$new_term->weight = $term->weight;
			$new_term->parent = $parent_tid;

			taxonomy_term_save($new_term);

			// Delete old reference
			taxonomy_menu_sync_delete_relation($term->tid, $item['mlid']);

			// Set new reference
			taxonomy_menu_sync_add_relation($new_term->tid, $item['mlid']);

			// Continue with item children
			if($children = taxonomy_term_sync_get_menu_item_children($item)){
				foreach($children as $child){
					_taxonomy_menu_sync_update_child_item($child, $new_term);
				}
			}
		}
	}
}

/**
 * @param array $item
 * @param array List of array menu items
 */
function taxonomy_term_sync_get_menu_item_children($item){
	$objects = db_select('menu_links', 'l')
		->fields('l', array('menu_name', 'mlid', 'plid', 'link_path', 'link_title', 'weight'))
		->condition('menu_name', $item['menu_name'])
		->condition('l.plid', $item['mlid'])
		->execute()
		->fetchAll();

	$list = array();
	foreach($objects as $object){
		$list[$object->mlid] = get_object_vars($object);
	}

	return $list;
}


function taxonomy_menu_update_menu_item_referenced_term($item, $term, $parent_tid = 0){
	$label = $item['link_title'];

	$original = $term;
	$return = $term;

	$relations = taxonomy_menu_get_term_relations($term->tid);
	$term_parent_term = taxonomy_menu_sync_get_parent_term($term->tid);
	// If menu item has changed its name or parent
	if($label != $term->name || (boolean) $parent_tid != (boolean) $term_parent_term || ($parent_tid && $term_parent_term && $parent_tid != $term_parent_term->tid)){
		$existing = taxonomy_menu_sync_search_existing_term($term->vid, $label, $parent_tid, $term->tid);
		if(!$existing){
			if(count($relations) == 1){
				// Update old term
				$term->name = $label;
				$term->parent = $parent_tid;
				$term->weight = $item['weight'];

				taxonomy_term_save($term);

				$return = $term;
			}else{
				// create a new term as a copy of original term
				$new_term = clone $term;
				unset($new_term->tid);
				$new_term->name = $label;
				$new_term->parent = $parent_tid;
				$new_term->weight = $item['weight'];

				taxonomy_term_save($new_term);

				// Delete old relation to this menu item
				taxonomy_menu_sync_delete_relation($term->tid, $item['mlid']);

				// Add new relation
				taxonomy_menu_sync_add_relation($new_term->tid, $item['mlid']);

				// Update menu item link
				taxonomy_menu_sync_update_menu_link($new_term->tid, $item['mlid']);

				// Continue with menu item children
				foreach(taxonomy_term_sync_get_menu_item_children($item) as $child){
					$child_term = taxonomy_term_sync_get_taxonomy_term($child['mlid']);
					taxonomy_menu_update_menu_item_referenced_term($child, $child_term, $new_term->tid);
				}

				$return = $new_term;
			}
		}else{
			// Delete old relation to this menu item
			taxonomy_menu_sync_delete_relation($term->tid, $item['mlid']);

			// Add new relation
			taxonomy_menu_sync_add_relation($existing->tid, $item['mlid']);

			// Update menu item link
			taxonomy_menu_sync_update_menu_link($existing->tid, $item['mlid']);

			// Continue with menu item children
			foreach(taxonomy_term_sync_get_menu_item_children($item) as $child){
				$child_term = taxonomy_term_sync_get_taxonomy_term($child['mlid']);
				taxonomy_menu_update_menu_item_referenced_term($child, $child_term, $existing->tid);
			}

			if(count($relations) == 1){
				// Let other modules modify anything about this new term
				module_invoke_all('taxonomy_term_sync_modified', $existing, $term);
				taxonomy_term_delete($term->tid);
			}

			$return = $existing;
		}
	}else{
		// It's just a weight change
		$term->weight = $item['weight'];
		taxonomy_term_save($term);
	}

	return $return;
}

/**
 * @param int $tid
 * @param int $mlid
 */
function taxonomy_menu_sync_update_menu_link($tid, $mlid){
	$router_path = db_select('menu_links', 'ml')
		->fields('ml', array(
			'router_path'
		))
		->condition('mlid', $mlid)
		->execute()
		->fetchField();

	if($router_path == 'taxonomy/term/%' || $router_path == 'sync_tax'){
		db_update('menu_links')
			->fields(array(
				'link_path' => 'taxonomy/term/'.$tid,
				'router_path' => 'taxonomy/term/%',
			))
			->condition('mlid', $mlid)
			->execute();
	}

	return true;
}

/**
 * @param object $vocabulary
 * @param string $machine_name
 * @return array|boolean
 */
function taxonomy_menu_sync_load_bundle($vocabulary, $machine_name){
	$bundles = variable_get('taxonomy_menu_sync_bundles_'.$vocabulary->machine_name, array());

	return isset($bundles[$machine_name]) ? $bundles[$machine_name] : false;
}

/**
 * @param object $term
 * @return array|boolean
 */
function taxonomy_menu_sync_get_term_bundle($term){
	$bundles = variable_get('taxonomy_menu_sync_bundles_'.$term->vocabulary_machine_name, array());
	foreach($bundles as $bundle){
		$field_name = $bundle['field'];
		$items = field_get_items('taxonomy_term', $term, $field_name);
		$item = reset($items);
		if($item['value'] == $bundle['value']){
			return $bundle;
		}
	}

	return false;
}

/**
 * @param array $bundle
 * @return array|boolean
 */
function taxonomy_menu_sync_get_bundle_settings($bundle){
	if(!isset($bundle['vocabulary']) || !isset($bundle['machine_name']) || !isset($bundle['field'])){
		return false;
	}

	$bundle_settings = variable_get('taxonomy_menu_sync_'.$bundle['vocabulary'].'__'.$bundle['machine_name'].'_data', array());
	$bundle_settings[$bundle['field']] = array(
		'mode' => 0,
		'default_value' => $bundle['value']
	);

	return $bundle_settings;
}

/**
 * @param object $term
 * @param array $bundle_settings
 * @param array $form
 */
function _taxonomy_menu_sync_alter_term_form($term, $bundle_settings, &$form, &$form_state){
	$langcode = entity_language('taxonomy_term', $term);
	$language = $langcode ? field_valid_language($langcode) : LANGUAGE_NONE;

	// Default values
	foreach($bundle_settings as $field_name => $data){
		// Only set default value if field is not defined
		if(isset($term->{$field_name}) || ($data['mode'] == 0 && empty($data['default_value']))){
			continue;
		}

		$field = field_info_field($field_name);
		switch($field['type']){
			case 'taxonomy_term_bundle_reference':
			case 'taxonomy_term_reference':
				$key = 'tid';
				break;

			case 'text':
			default:
				$key = 'value';
				break;
		}
		$term->{$field_name} = array(
			$language => array(
				0 => array(
					$key => $data['default_value']
				)
			)
		);
	}
	$form['original_term'] = array(
		'#type' => 'value',
		'#value' => $term,
	);
	field_attach_form('taxonomy_term', $term, $form, $form_state, $langcode);

	// Alter fields with bundle_settings
	foreach($bundle_settings as $field_name => $data){
		if(!isset($form[$field_name])){
			continue;
		}

		$field = field_info_field($field_name);
		switch($field['type']){
			case 'taxonomy_term_bundle_reference':
			case 'taxonomy_term_reference':
				$key = 'tid';
				break;

			case 'text':
			default:
				$key = 'value';
				break;
		}

		switch($data['mode']){
			// Disabled
			case 0:
				if(empty($data['default_value'])){
					unset($form[$field_name]);
				}else{
					$form[$field_name]['#access'] = false;
				}
				break;

			// Visible
			case 1:
				break;

			// Required
			case 2:
				$form[$field_name][$language][0][$key]['#required'] = true;
				break;
		}
	}
}

/**
 * @param object $vocabulary
 * @param string $bundle_machine_name
 * @return array
 */
function taxonomy_menu_sync_get_bundle_terms($vocabulary, $bundle_machine_name){
	$bundle_terms = array();

	$terms = entity_load('taxonomy_term', FALSE, array('vid' => $vocabulary->vid));
	foreach($terms as $term){
		$term_bundle = taxonomy_menu_sync_get_term_bundle($term);
		if($term_bundle && $term_bundle['machine_name'] == $bundle_machine_name){
			$bundle_terms[$term->tid] = $term;
		}
	}

	return $bundle_terms;
}