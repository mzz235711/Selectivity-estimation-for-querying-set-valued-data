select count(*) from sample where A<=2001 and B="mall" and C<@'{"can":0,"meat":0,"dairy":0,"snake";0}';
select count(*) from sample where A=2002 and B="supermarket" and C@>'{"vegetable":0}';