# Decision Log

## Template
- Date:
- Time (UTC+8):
- Decision:
- Rationale:
- Alternatives:
- Impact:

## Entries (newest first)
- Date: 2026-02-09
  Time (UTC+8): 14:00
  Decision: Complete Phase 0 - Firebase backend setup with Storage security rules.
  Rationale: Configured Firebase project with Storage, Authentication (Anonymous), and deployed security rules. Rules ensure only authenticated users can upload to /audio/* and prevent direct downloads (signed URLs only via Cloud Functions). This establishes secure foundation for mobile app uploads.
  Alternatives: Could have used Firebase Authentication with Google Sign-In, but Anonymous auth is faster for MVP and can be upgraded later.
  Impact: Backend infrastructure ready for Phase 1 (Flutter app). Storage security enforced at Firebase level. Setup documentation created for reproducibility.

- Date: 2026-02-09
  Time (UTC+8): 12:00
  Decision: Bootstrap project with organized folder structure for mobile, functions, firmware, and docs.
  Rationale: Establish clear separation of concerns from the start; each component (Flutter app, Cloud Functions, ESP32 firmware) lives in its own directory with dedicated READMEs. Documentation directory provides centralized architecture, setup, security, and API references.
  Alternatives: Keep flat structure or wait until components are implemented to create folders.
  Impact: Clear project organization makes onboarding easier, reduces confusion, and follows best practices for multi-component projects. Sets foundation for parallel development of each component.

- Date: 2026-02-09
  Time (UTC+8): 00:00
	Decision: Standardize decision log timestamps to UTC+8.
	Rationale: Align timestamps with preferred timezone (Taipei).
	Alternatives: Keep times without timezone.
	Impact: Clearer time context for future entries.

- Date: 2026-02-09
	Time (UTC+8): 00:00
	Decision: Expand .gitignore to cover Flutter, Firebase Functions, ESP-IDF, and common tooling.
	Rationale: Prevent build artifacts and secrets from being committed.
	Alternatives: Keep minimal .gitignore.
	Impact: Cleaner repo and safer defaults.

- Date: 2026-02-09
	Time (UTC+8): 00:00
	Decision: Use CODEOWNERS as the reviewer source in workflow instructions.
	Rationale: Avoid hardcoding a specific username in instructions.
	Alternatives: Keep explicit @mention in instructions.
	Impact: Reviewer assignment stays centralized.

- Date: 2026-02-09
	Time (UTC+8): 00:00
	Decision: Standardize AI branch/PR workflow for each project step.
	Rationale: Ensure traceability and consistent review by @Vellyxenya.
	Alternatives: Manual branching and reviewer assignment.
	Impact: Clearer history and reliable review flow.

- Date: 2026-02-09
	Time (UTC+8): 00:00
	Decision: Require updating tasks.md on completion in workflow instructions.
	Rationale: Ensure progress tracking is consistently maintained.
	Alternatives: Rely on ad-hoc updates.
	Impact: Tasks board stays current after each change.

- Date: 2026-02-09
	Time (UTC+8): 00:00
	Decision: Merge workflow guidance into project instructions and remove workflow.md.
	Rationale: Ensure workflow rules always apply via custom instructions.
	Alternatives: Keep workflow as standalone documentation only.
	Impact: Single source of truth for workflow guidance.

- Date: 2026-02-09
	Time (UTC+8): 00:00
	Decision: Update workflow artifacts list to match current .github structure.
	Rationale: Keep workflow documentation aligned with actual files.
	Alternatives: Leave outdated artifact list.
	Impact: Reduces confusion and improves discoverability.

- Date: 2026-02-09
	Time (UTC+8): 00:00
	Decision: Move full project plan into Backlog section.
	Rationale: Treat the plan as the initial backlog for execution tracking.
	Alternatives: Keep the plan as a separate section outside progress tracking.
	Impact: All tasks now live under Backlog for easy status updates.

- Date: 2026-02-09
	Time (UTC+8): 00:00
	Decision: Reintroduce progress board sections in tasks.
	Rationale: Track backlog, in-progress, and completed work alongside the plan.
	Alternatives: Keep only the phased plan without status tracking.
	Impact: Enables ongoing progress management.

- Date: 2026-02-09
	Time (UTC+8): 00:00
	Decision: Populate tasks with full project plan for audio notification system.
	Rationale: Establishes a shared, phase-based roadmap for implementation.
	Alternatives: Keep tasks as a minimal backlog only.
	Impact: Clear execution steps across Flutter, Firebase, MQTT, and ESP32.

- Date: 2026-02-09
	Time (UTC+8): 00:00
	Decision: Store AI guidance in .github with prompt files under .github/prompts.
	Rationale: Aligns with VS Code custom instructions and prompt file conventions.
	Alternatives: Keep root-level guidance files only.
	Impact: Copilot can auto-discover instructions; prompt files are opt-in.
