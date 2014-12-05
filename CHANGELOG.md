# Taxonomy Menu Sync

## Version 1.x-dev

- Synchronized menus ordered by weight  
Taxonomy terms are ordered in field widget using synchronized menu weights

## Version 1.2

Date_ 2014/12/04

- Hook `menu_item_element_bundle_operations`  
Allow other modules to add operations over a menu item when it has a referenced taxonmy term of a defined taxonomy bundle.

- Module *tms_field*  
	- Field type *taxonomy bundle term reference*
	- Widget with radios or checkboxes. Depends on field cardinality.

## Version 1.1

Date: 2014/12/03

- Configure default actions  
Clean default actions on menu items from content menu module.

- Taxonomy bundles  
Define subbundles inside a taxonomy vocabulary.  
Create any subbundle term from menu management.

- Taxonomy form included in menu item edit form  
Fields filtered by taxonomy bundle.

## Version 1.0 

Date: 2014/11/26

- Link menus with taxonomy vocabularies  
It is allowed to get multiple menus synchronized with a single taxonomy vocabulary

- It is not allowed to remove menu items with children

- Content menu task 'New section' in stead of 'Placeholder'