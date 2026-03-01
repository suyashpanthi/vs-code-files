"""
Development Agent Team using Claude Agent SDK

A multi-agent system with three specialized subagents:
  - Architect: Plans the implementation approach and file structure
  - Developer: Writes and edits code based on the architect's plan
  - Reviewer: Reviews the code for bugs, quality, and improvements

Usage:
    python dev_team.py "Add a REST API endpoint for user registration"
    python dev_team.py --cwd /path/to/project "Refactor the auth module"
"""

import sys
import anyio
from claude_agent_sdk import query, ClaudeAgentOptions, AgentDefinition, ResultMessage


AGENTS = {
    "architect": AgentDefinition(
        description="Software architect that analyzes requirements and creates implementation plans.",
        prompt=(
            "You are a software architect. When given a task:\n"
            "1. Explore the codebase to understand the existing structure\n"
            "2. Identify which files need to be created or modified\n"
            "3. Outline the implementation steps in order\n"
            "4. Flag any risks or dependencies\n"
            "Be specific about file paths and function signatures."
        ),
        tools=["Read", "Glob", "Grep"],
    ),
    "developer": AgentDefinition(
        description="Developer that writes and edits code following the architect's plan.",
        prompt=(
            "You are a developer. When given a task:\n"
            "1. Follow the plan provided by the architect\n"
            "2. Write clean, well-structured code\n"
            "3. Use existing patterns and conventions from the codebase\n"
            "4. Keep changes minimal and focused\n"
            "Do not add unnecessary abstractions or features beyond what was asked."
        ),
        tools=["Read", "Write", "Edit", "Glob", "Grep", "Bash"],
    ),
    "reviewer": AgentDefinition(
        description="Code reviewer that checks for bugs, security issues, and quality.",
        prompt=(
            "You are a senior code reviewer. When given code to review:\n"
            "1. Check for bugs and logic errors\n"
            "2. Look for security vulnerabilities (injection, XSS, etc.)\n"
            "3. Verify error handling is adequate\n"
            "4. Suggest concrete improvements with specific line references\n"
            "Be constructive and prioritize issues by severity."
        ),
        tools=["Read", "Glob", "Grep"],
    ),
}


async def run_dev_team(task: str, cwd: str = "."):
    """Run the development team on a task."""
    options = ClaudeAgentOptions(
        cwd=cwd,
        allowed_tools=["Read", "Write", "Edit", "Glob", "Grep", "Bash", "Agent"],
        agents=AGENTS,
        system_prompt=(
            "You are a tech lead coordinating a development team. You have three agents:\n"
            "- architect: Plans the implementation (use first)\n"
            "- developer: Writes the code (use after architect)\n"
            "- reviewer: Reviews the result (use after developer)\n\n"
            "For each task:\n"
            "1. Ask the architect to analyze and plan\n"
            "2. Ask the developer to implement the plan\n"
            "3. Ask the reviewer to check the result\n"
            "4. If the reviewer finds issues, ask the developer to fix them\n"
            "5. Summarize what was done"
        ),
        max_turns=30,
    )

    print(f"Task: {task}\n")
    print("Starting development team...\n")

    async for message in query(prompt=task, options=options):
        if isinstance(message, ResultMessage):
            print("\n--- Result ---")
            print(message.result)


def main():
    if len(sys.argv) < 2:
        print("Usage: python dev_team.py [--cwd /path] \"task description\"")
        sys.exit(1)

    cwd = "."
    args = sys.argv[1:]

    if args[0] == "--cwd" and len(args) >= 3:
        cwd = args[1]
        task = " ".join(args[2:])
    else:
        task = " ".join(args)

    anyio.run(lambda: run_dev_team(task, cwd))


if __name__ == "__main__":
    main()
