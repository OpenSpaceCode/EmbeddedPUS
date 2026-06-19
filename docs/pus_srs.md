# Minimal ECSS PUS Implementation – Software Requirements Specification (SRS)

## 1. Introduction

### 1.1 Purpose
This document defines the software requirements for a minimal implementation of the ECSS Packet Utilisation Standard (PUS) in C, intended for small-scale and educational space missions.

### 1.2 Scope
The library provides:
- Telecommand (TC) processing
- Telemetry (TM) generation
- Minimal subset of PUS services

It is designed to integrate with existing CCSDS Space Packet and lower-layer protocols.

---

## 2. References
- ECSS-E-ST-70-41 (PUS)
- CCSDS Space Packet Standard

---

## 3. System Overview

The system is a lightweight C library operating on byte buffers, without transport layer responsibility.

Target platforms:
- Embedded systems (ARM Cortex-M)
- Desktop (Linux)

---

## 4. Functional Requirements

### 4.1 Telecommand Processing
The system shall:
- Accept CCSDS Space Packets as input
- Validate packet structure (APID, length, optional CRC)
- Extract PUS service type and subtype

### 4.2 Telecommand Routing
The system shall:
- Route TC packets to registered handlers based on service and subtype

The system should:
- Allow dynamic handler registration

### 4.3 Telemetry Generation
The system shall:
- Generate TM packets compliant with CCSDS and PUS
- Support both automatic and application-level telemetry

### 4.4 Service 1 – Telecommand Verification
The system shall:
- Generate verification reports:
  - Acceptance
  - Execution start
  - Execution completion
  - Failure

### 4.5 Service 3 – Housekeeping
The system shall:
- Provide parameter reporting

The system should:
- Support periodic and on-request reporting

### 4.6 Service 5 – Event Reporting
The system shall:
- Support event generation (info, warning, error)

### 4.7 Service 17 – Test
The system shall:
- Support ping command with response

### 4.8 Error Handling
The system shall:
- Detect invalid packets and unknown services
- Generate appropriate error telemetry

---

### 5 Integration
The system shall:
- Operate on raw byte buffers
- Not implement transport layer functionality

---

## 6. Non-Functional Requirements

### 6.1 Language
- C11 compliant

### 6.2 Memory
- No dynamic allocation

### 6.3 Determinism
- No blocking operations
- Predictable execution time

### 6.4 Portability
- Platform-independent design

### 6.5 Resource Constraints
- RAM usage: ~10–50 KB
- Minimal FLASH footprint


---

## 7. Testing

The system shall:
- Include unit tests
- Provide example TC/TM packets

The system should:
- Include conformance test vectors

---

## 8. Out of Scope
- Full ECSS compliance
- Security mechanisms
- GUI tools
- On-board storage management

---

## 9. Design Philosophy

The implementation should follow:
- Minimalism
- Modularity
- Clear separation between protocol handling and application logic

