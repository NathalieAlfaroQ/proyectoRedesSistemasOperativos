# Configuración de los Switches

En terminal:

```bash
sudo minicom
```

## Configuración del Switch L3

Revisar que el switch no tiene una configuración de alguien:

```bash
Switch>en
Switch#erase startup-config
Switch#reload
```

No hacer autoinstall.

Para copiar la configuración, entrar en modo de configuración:

```bash
Switch#conf t
```

Extraer el texto con la configuración:

```bash
!
version 12.2(37)SE1
service timestamps log datetime msec
service timestamps debug datetime msec
no service password-encryption
!
hostname L3-Switch
!
!
!
!
!
!
ip routing
!
!
!
!
!
!
!
!
!
!
!
!
!
!
!
spanning-tree mode pvst
!
!
!
!
!
!
interface FastEthernet0/1
 switchport trunk native vlan 110
 switchport trunk allowed vlan 110
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/2
 switchport trunk native vlan 120
 switchport trunk allowed vlan 120
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/3
 switchport trunk native vlan 130
 switchport trunk allowed vlan 130
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/4
 switchport trunk native vlan 140
 switchport trunk allowed vlan 140
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/5
 switchport trunk native vlan 150
 switchport trunk allowed vlan 150
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/6
 switchport trunk native vlan 160
 switchport trunk allowed vlan 160
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/7
!
interface FastEthernet0/8
!
interface FastEthernet0/9
!
interface FastEthernet0/10
!
interface FastEthernet0/11
!
interface FastEthernet0/12
!
interface FastEthernet0/13
!
interface FastEthernet0/14
!
interface FastEthernet0/15
!
interface FastEthernet0/16
!
interface FastEthernet0/17
!
interface FastEthernet0/18
!
interface FastEthernet0/19
!
interface FastEthernet0/20
!
interface FastEthernet0/21
!
interface FastEthernet0/22
!
interface FastEthernet0/23
!
interface FastEthernet0/24
!
interface GigabitEthernet0/1
!
interface GigabitEthernet0/2
!
interface Vlan1
 no ip address
 shutdown
!
interface Vlan110
 mac-address 00e0.f905.6401
 ip address 172.16.123.17 255.255.255.240
!
interface Vlan120
 mac-address 00e0.f905.6402
 ip address 172.16.123.33 255.255.255.240
!
interface Vlan130
 mac-address 00e0.f905.6403
 ip address 172.16.123.49 255.255.255.240
!
interface Vlan140
 mac-address 00e0.f905.6404
 ip address 172.16.123.65 255.255.255.240
!
interface Vlan150
 mac-address 00e0.f905.6405
 ip address 172.16.123.81 255.255.255.240
!
interface Vlan160
 mac-address 00e0.f905.6406
 ip address 172.16.123.97 255.255.255.240
!
ip classless
!
ip flow-export version 9
!
!
!
!
!
!
!
!
line con 0
 login
!
line aux 0
!
line vty 0 4
 login
line vty 5 15
 login
!
!
!
!
end

```

## Crear VLANs

La configuración no tiene las VLANs, entonces hay que hacerlas, en modo configuración:

```bash
vlan 110
#name Grupo1
exit
vlan 120
#name Grupo2
exit
vlan 130
#name Grupo3
exit
vlan 140
#name Grupo4
exit
vlan 150
#name Grupo5
exit
vlan 160
#name Grupo6
exit
```

## Configuración del Switch L2

En modo configuración, rellenar los espacios entre <>, con los datos de cada grupo:

```bash
!
version 12.1
no service timestamps log datetime msec
no service timestamps debug datetime msec
no service password-encryption
!
hostname isla4-grupo1
!
!
ip dhcp excluded-address 172.16.123.65
ip dhcp excluded-address 172.16.123.66
!
ip dhcp pool VLAN140_POOL
 network 172.16.123.64 255.255.255.240
 default-router 172.16.123.65
!
!
!
spanning-tree mode pvst
spanning-tree extend system-id
!
interface FastEthernet0/1
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/2
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/3
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/4
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/5
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/6
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/7
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/8
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/9
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/10
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/11
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/12
 switchport access vlan 140
 switchport mode access
!
interface FastEthernet0/13
 switchport mode dynamic auto
!
interface FastEthernet0/14
 switchport mode dynamic auto
!
interface FastEthernet0/15
 switchport mode dynamic auto
!
interface FastEthernet0/16
 switchport mode dynamic auto
!
interface FastEthernet0/17
 switchport mode dynamic auto
!
interface FastEthernet0/18
 switchport mode dynamic auto
!
interface FastEthernet0/19
 switchport mode dynamic auto
!
interface FastEthernet0/20
 switchport mode dynamic auto
!
interface FastEthernet0/21
 switchport mode dynamic auto
!
interface FastEthernet0/22
 switchport mode dynamic auto
!
interface FastEthernet0/23
 switchport mode dynamic auto
!
interface FastEthernet0/24
 switchport mode dynamic auto
!
interface GigabitEthernet0/1
 switchport trunk native vlan 140
 switchport trunk allowed vlan 140
 switchport mode trunk
!
interface GigabitEthernet0/2
!
interface Vlan1
 no ip address
 shutdown
!
interface Vlan140
 ip address 172.16.123.66 255.255.255.240
!
!
!
!
line con 0
!
line vty 0 4
 login
line vty 5 15
 login
!
!
!
!
end
```

## Crear VLAN

Luego crear la VLAN manualmente:

```bash
vlan 140
#name Grupo4
exit
```