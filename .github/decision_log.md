# Decision Log

## Template
- Date:
- Decision:
- Rationale:
- Alternatives:
- Impact:

## Entries
- Date: 2026-02-09
	Decision: Store AI guidance in .github with prompt files under .github/prompts.
	Rationale: Aligns with VS Code custom instructions and prompt file conventions.
	Alternatives: Keep root-level guidance files only.
	Impact: Copilot can auto-discover instructions; prompt files are opt-in.

- Date: 2026-02-09
	Decision: Populate tasks with full project plan for audio notification system.
	Rationale: Establishes a shared, phase-based roadmap for implementation.
	Alternatives: Keep tasks as a minimal backlog only.
	Impact: Clear execution steps across Flutter, Firebase, MQTT, and ESP32.

- Date: 2026-02-09
	Decision: Reintroduce progress board sections in tasks.
	Rationale: Track backlog, in-progress, and completed work alongside the plan.
	Alternatives: Keep only the phased plan without status tracking.
	Impact: Enables ongoing progress management.

- Date: 2026-02-09
	Decision: Move full project plan into Backlog section.
	Rationale: Treat the plan as the initial backlog for execution tracking.
	Alternatives: Keep the plan as a separate section outside progress tracking.
	Impact: All tasks now live under Backlog for easy status updates.

- Date: 2026-02-09
	Decision: Update workflow artifacts list to match current .github structure.
	Rationale: Keep workflow documentation aligned with actual files.
	Alternatives: Leave outdated artifact list.
	Impact: Reduces confusion and improves discoverability.

- Date: 2026-02-09
	Decision: Merge workflow guidance into project instructions and remove workflow.md.
	Rationale: Ensure workflow rules always apply via custom instructions.
	Alternatives: Keep workflow as standalone documentation only.
	Impact: Single source of truth for workflow guidance.

- Date: 2026-02-09
	Decision: Require updating tasks.md on completion in workflow instructions.
	Rationale: Ensure progress tracking is consistently maintained.
	Alternatives: Rely on ad-hoc updates.
	Impact: Tasks board stays current after each change.

- Date: 2026-02-09
	Decision: Expand .gitignore to cover Flutter, Firebase Functions, ESP-IDF, and common tooling.
	Rationale: Prevent build artifacts and secrets from being committed.
	Alternatives: Keep minimal .gitignore.
	Impact: Cleaner repo and safer defaults.
