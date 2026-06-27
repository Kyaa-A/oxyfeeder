# OxyFeeder: Process Workflow Guide

## 1. Development Workflow
- AI-assisted generation → Dev implementation → Peer review → QA testing → Deployment → Iteration.  
- Use **Cursor** + **Vertex AI Studio** for AI-generated logic and documentation.

## 2. Roles & Responsibilities
- **PM:** Timeline & PRD owner  
- **Embedded Dev:** Firmware (Arduino, ESP32)  
- **Flutter Dev:** Mobile app  
- **QA Engineer:** Test plans & validation  
- **AI Assistant:** Drafting, debugging, documentation  

## 3. Task Lifecycle
Backlog → To Do → In Progress → In Review → QA → Done  

## 4. Tools
| Tool | Purpose | Role |
|------|----------|------|
| GitHub | Version control, PRs, boards | All |
| Cursor | AI coding IDE | Flutter Dev |
| Arduino IDE / PlatformIO | Firmware development | Embedded Dev |
| Vertex AI Studio | Logic + doc generation | All |
| Flutter SDK | App framework | Flutter Dev |

## 5. Commit Rules
- **Conventional Commits** (feat, fix, docs, refactor, etc.)  
- Reference the **Task ID** (e.g., `feat(firmware): add DO sensor reading #FIRM-01`).  

## 6. Weekly Progress Checklist
- Review “Done” tasks  
- Identify blockers  
- Demo new features  
- Plan next sprint  
- Verify documentation updates  
